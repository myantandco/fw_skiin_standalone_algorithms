import csv

import numpy as np
from scipy.signal import butter, cont2discrete, lfilter, lfiltic

dtype = np.float32


def logit(x):
    return np.log(x / (1 - x))


def sigmoid(x):
    return 1 / (1 + np.exp(-x))


class LowpassSubtract:
    def __init__(self, tau):
        self.tau = tau
        self.alpha = np.exp(-1 / self.tau)
        self.b = np.array([1 - self.alpha], dtype=dtype)
        self.a = np.array([1, -self.alpha], dtype=dtype)

    def __call__(self, x):
        return x - lfilter(self.b, self.a, x, axis=0)


class Notch:
    def __init__(self, f0=59, f1=61, fs=320):
        self.f0 = f0
        self.f1 = f1
        b, a = butter(2, [f0 / (fs / 2), f1 / (fs / 2)], "bandstop")
        self.b = b.astype(dtype)
        self.a = a.astype(dtype)

    def __call__(self, x):
        return lfilter(self.b, self.a, x, axis=0)


class QualityFilter:
    def __init__(
        self,
        prefilter=dict(freqs=(5, 30)),
        power_filter=dict(freqs=2),
        softness=0.2,
        presoft=True,
        mask_ab=(0.15, 0.9),
        quality_ab=(4.71, 0.15),
        fs=320,
        nc=3,
    ):
        self.prefilter = prefilter
        self.power_filter = power_filter
        self.mask_ab = mask_ab
        self.softness = softness
        self.presoft = presoft
        self.quality_ab = quality_ab
        self.fs = fs
        self.nc = nc

        self.bp, self.ap = self.create_butter(self.prefilter, default_kind="bandpass")
        self.b, self.a = self.create_butter(self.power_filter, default_kind="lowpass")
        if self.softness is not None:
            tau = int(round(self.softness * self.fs))
            self.bsoft = np.ones(tau) / tau
            self.asoft = np.ones(1)
            self.zsoft = np.column_stack(
                [
                    lfiltic(self.bsoft, self.asoft, np.ones(0), np.ones(tau - 1))
                    for _ in range(self.nc)
                ]
            )

    def create_butter(self, filt, default_kind, default_order=1):
        assert set(filt.keys()).issubset(("order", "freqs", "kind"))
        freqs = filt.get("freqs")
        order = filt.get("order", default_order)
        kind = filt.get("kind", default_kind)
        return butter(order, freqs, kind, fs=self.fs)

    def __call__(self, x):
        assert x.shape[-1] == self.nc

        x_bp = lfilter(self.bp, self.ap, x, axis=0)
        q = x - x_bp
        x = x_bp

        q = lfilter(self.b, self.a, np.abs(q), axis=0)

        a, b = self.mask_ab
        qm = 1 - self._latch_signal(q, a, b)
        if self.softness is not None:
            qm_filt, _ = lfilter(self.bsoft, self.asoft, qm, axis=0, zi=self.zsoft)
            if not self.presoft:
                qm_filt[qm == 0] = 0
            qm = qm_filt

        x = x * qm

        if self.quality_ab is not None:
            a, b = self.quality_ab
            q = sigmoid(a * (q - b))

        return x, q

    @staticmethod
    def _latch_signal(x, x0, x1):
        """If signal goes above `x1`, output 1 until it drops below `x0`."""
        nt, nc = x.shape

        xt = np.zeros_like(x)
        xt[(x >= x1)] = 1
        xt[(x <= x0)] = -2

        yt = np.diff(xt, axis=0, prepend=0)
        (ti, tc) = ((yt >= 1) & (xt >= 1) | (yt <= -2) & (xt <= -2)).nonzero()

        t = np.arange(nt)
        y = np.zeros_like(x)
        for c in range(nc):
            tic = ti[tc == c]

            ytc = yt[tic, c]
            yic = np.zeros(tic.shape, dtype=x.dtype)
            yic[ytc >= 1] = 1

            tic = np.concatenate([[-1], tic])
            yic = np.concatenate([[1 if x[0, c] >= x1 else 0], yic])

            i = np.searchsorted(tic, t, side="right")
            y[:, c] = yic[i - 1]

        return y


class Clip:
    def __init__(self, low, high):
        self.low = low
        self.high = high

    def __call__(self, x):
        return np.clip(x, self.low, self.high)


class RpeakDetector:
    def __init__(self, threshold, delay, fs):
        self.threshold = threshold
        self.delay = delay
        self.refractory_period = int(0.5 * (60 / 220) * fs)  # half period of 220 bpm

    def __call__(self, x):
        if x.ndim == 2:
            x = x.squeeze(axis=-1)

        assert x.ndim == 1
        peaks = np.where(x > self.threshold)[0]
        peaks = apply_max_peak(
            peaks,
            outputs=x,
            min_ref=self.refractory_period,
        )
        peaks = peaks - self.delay
        peaks = peaks[(peaks >= 0) & (peaks < len(x))]
        return peaks


def apply_refractory_period(peaks, refractory_period):
    """Detects an R-peak when output crosses threshold, then prohibits for a period."""
    if len(peaks) == 0:
        return np.array([], dtype=peaks.dtype)

    filtered_peaks = [peaks[0]]
    for peak in peaks:
        if peak - filtered_peaks[-1] > refractory_period:
            filtered_peaks.append(peak)

    return np.array(filtered_peaks, dtype=peaks.dtype)


def apply_max_peak(peaks, outputs, min_width=1, min_ref=1):
    """Finds the peak with the maximum value in a contiguous section of peaks."""
    filtered_peaks = []
    max_peak = None  # output value
    max_peak_timestep = None  # timestep
    width = 0
    last_peak = None
    ref = 0  # refractory time (timesteps spent below threshold)

    for peak in peaks:
        out = outputs[peak]
        diff = 0 if last_peak is None else peak - last_peak

        if diff > 1:
            ref += diff - 1

            if ref >= min_ref:
                # last region has ended
                if width >= min_width:
                    filtered_peaks.append(max_peak_timestep)

                # start new region
                max_peak = None
                max_peak_timestep = None
                width = 0
                ref = 0

        # update current region
        if max_peak is None or out > max_peak:
            max_peak = out
            max_peak_timestep = peak

        width += 1
        last_peak = peak

    if max_peak is not None and width >= min_width:
        filtered_peaks.append(max_peak_timestep)

    return np.array(filtered_peaks, dtype=peaks.dtype)


class RpeakEvaluator:
    def __init__(self, detection_period=25, fs=320):
        self.detection_period = int(detection_period * 1e-3 * fs)
        # self.distance_threshold = int((60 / 30) * fs)  # period of 30 bpm

    def __call__(self, true_beats, detected_beats, noisy_mask=None):
        cm = BinaryConfusion()
        clean_cm = BinaryConfusion()
        noisy_cm = BinaryConfusion()

        max_t = max(
            true_beats[-1] if len(true_beats) > 0 else -1,
            detected_beats[-1] if len(detected_beats) > 0 else -1,
        )
        noisy_mask = (
            np.zeros(max_t + 1, dtype=bool) if noisy_mask is None else noisy_mask
        )
        assert len(noisy_mask) > max_t

        # Simultaneously sweep both the filtered beats and the ideal beats to classify
        #  true positives on the basis of being within the detection_period ("nearby"),
        #  false positives on the basis of there being no ideal peak nearby, or
        #  false negatives on the basis of there being no filtered peak nearby.
        j = 0
        k = 0
        while j < len(true_beats) and k < len(detected_beats):
            tb = true_beats[j]
            db = detected_beats[k]
            # tb0 = tb if j == 0 else true_beats[j - 1]
            # cur_cm = noisy_cm if noisy_mask[tb] else clean_cm
            # cur_cm0 = noisy_cm if noisy_mask[tb0] else clean_cm

            cur_cm = noisy_cm if noisy_mask[tb] else clean_cm
            cur_cm0 = noisy_cm if noisy_mask[db] else clean_cm

            if abs(db - tb) <= self.detection_period:
                cm.tp += 1
                cur_cm.tp += 1
                j += 1
                k += 1
            elif db < tb:
                cm.fp += 1
                # (cur_cm0 if abs(db - tb0) < abs(db - tb) else cur_cm).fp += 1
                cur_cm0.fp += 1
                k += 1
            else:
                assert db > tb
                cm.fn += 1
                cur_cm.fn += 1
                j += 1

        while j < len(true_beats):
            cm.fn += 1
            (noisy_cm if noisy_mask[true_beats[j]] else clean_cm).fn += 1
            j += 1

        while k < len(detected_beats):
            cm.fp += 1
            (noisy_cm if noisy_mask[detected_beats[k]] else clean_cm).fp += 1
            k += 1

        cm.fn += len(true_beats) - j  # fast-forward to the end of the true signal
        cm.fp += len(detected_beats) - k  # fast-forward to the end of the output signal
        assert cm.n_true == len(true_beats)
        assert cm.n_predicted == len(detected_beats)

        return dict(total=cm, noisy=noisy_cm, clean=clean_cm)


class QualityEvaluator:
    def __init__(self, threshold=0.5):
        self.threshold = threshold

    def __call__(self, true_quality, pred_quality):
        pred_quality = pred_quality > self.threshold
        correct = true_quality == pred_quality

        return BinaryConfusion(
            tp=correct[true_quality].sum(),
            tn=correct[~true_quality].sum(),
            fn=(~correct)[true_quality].sum(),
            fp=(~correct)[~true_quality].sum(),
        )


class BinaryConfusion:
    """Evaluation metrics for a {TP, FP, FN} confusion matrix."""

    __builtin_sum = sum

    def __init__(self, tp=0, fp=0, fn=0, tn=None):
        if tp < 0 or fp < 0 or fn < 0 or tn is not None and tn < 0:
            raise ValueError(f"counts ({tp}, {fp}, {fn}, {tn}) must be non-negative")
        if int(tp) != tp or int(fp) != fp or int(fn) != fn:
            raise TypeError(f"counts ({tp}, {fp}, {fn}) must be integers")

        self.tp = int(tp)
        self.fp = int(fp)
        self.fn = int(fn)
        self.tn = int(tn) if tn is not None else None

    @property
    def n_predicted(self):
        """Number of examples that were classified as true."""
        return self.tp + self.fp

    @property
    def precision(self):
        """Fraction of examples predicted true that are actually correct."""
        try:
            return self.tp / self.n_predicted
        except ZeroDivisionError:
            return np.nan

    @property
    def n_true(self):
        """Number of ground-truth examples that are to be recalled."""
        return self.tp + self.fn

    @property
    def sensitivity(self):
        """Fraction of ground-truth examples that are classified as true (recall)."""
        try:
            return self.tp / self.n_true
        except ZeroDivisionError:
            return np.nan

    @property
    def f1(self):
        """Harmonic mean of precision and sensitivity."""
        try:
            return self.tp / (self.tp + 0.5 * (self.fp + self.fn))
        except ZeroDivisionError:
            return np.nan

    @property
    def accuracy(self):
        assert self.tn is not None
        try:
            return (self.tp + self.tn) / (self.tp + self.tn + self.fp + self.fn)
        except ZeroDivisionError:
            return np.nan

    @property
    def neg_sensitivity(self):
        assert self.tn is not None
        try:
            return self.tn / (self.tn + self.fp)
        except ZeroDivisionError:
            return np.nan

    @property
    def neg_precision(self):
        assert self.tn is not None
        try:
            return self.tn / (self.tn + self.fn)
        except ZeroDivisionError:
            return np.nan

    @property
    def neg_f1(self):
        assert self.tn is not None
        try:
            return self.tn / (self.tn + 0.5 * (self.fp + self.fn))
        except ZeroDivisionError:
            return np.nan

    def __add__(self, other):
        if other == 0:
            return self
        if not isinstance(other, BinaryConfusion):
            return NotImplemented
        return BinaryConfusion(
            tp=self.tp + other.tp,
            fp=self.fp + other.fp,
            fn=self.fn + other.fn,
            tn=None if self.tn is None or other.tn is None else self.tn + other.tn,
        )

    def __radd__(self, other):
        return self.__add__(other)

    @classmethod
    def sum(cls, confusion_matrices):
        """Adds together a list of BinaryConfusion matrices."""
        return cls.__builtin_sum(confusion_matrices)

    def __repr__(self):
        return f"{type(self).__name__}(tp={self.tp}, fp={self.fp}, fn={self.fn}, tn={self.tn})"

    def __str__(self):
        return repr(self)

    def __eq__(self, other):
        if not isinstance(other, BinaryConfusion):
            return False
        return (
            self.tp == other.tp
            and self.fp == other.fp
            and self.fn == other.fn
            and self.tn == other.tn
        )


def results_to_csv(csv_filename, dataset, results):
    with open(csv_filename, "w", newline="") as csvfile:
        csvwriter = csv.writer(csvfile, delimiter=",")

        suffix_map = dict(total="", noisy="_n", clean="_c")

        col_names = [
            "Index",
            "Split(s)",
            "Name",
        ]
        for key, suffix in suffix_map.items():
            col_names.extend(
                [
                    f"TP{suffix}",
                    f"FP{suffix}",
                    f"FN{suffix}",
                    f"Sen{suffix}",
                    f"Pre{suffix}",
                    f"F1{suffix}",
                    # f"F1{suffix} ({threshold_type}_th={best_threshold:0.3f})",
                ]
            )

        csvwriter.writerow(col_names)
        for i, result in enumerate(results):
            row = [
                i,
                ";".join(dataset.splits[i]),
                dataset.names[i],
            ]
            for key in suffix_map:
                cm = result[key]
                row.extend(
                    [
                        cm.tp,
                        cm.fp,
                        cm.fn,
                        cm.sensitivity,
                        cm.precision,
                        cm.f1,
                    ]
                )

            csvwriter.writerow(row)


def results_summary_to_csv(csv_filename, dataset, results, is_waist=False):
    groups = (
        ("All", "PACE", "TXTIT") if is_waist else ("All", "MIT", "PACE", "StP", "TIT")
    )

    with open(csv_filename, "w", newline="") as csvfile:
        csvwriter = csv.writer(csvfile, delimiter=",")

        suffix_map = dict(total="", noisy="_n", clean="_c")

        col_names = [
            "Name",
        ]
        for key, suffix in suffix_map.items():
            col_names.extend(
                [
                    f"Sen{suffix}",
                    f"Pre{suffix}",
                    f"F1{suffix}",
                ]
            )
        csvwriter.writerow(col_names)

        for kind in ("gross", "mean"):
            for group in groups:
                for split in ("all", "tvt", "train", "validation", "test", "ignore"):
                    inds = [i for i in range(len(dataset))]
                    if group != "All":
                        inds = [i for i in inds if dataset.names[i].startswith(group)]
                    if split == "tvt":
                        tvt_splits = ("train", "validation", "test")
                        inds = [
                            i
                            for i in inds
                            if any(s in dataset.splits[i] for s in tvt_splits)
                        ]
                    else:
                        inds = [i for i in inds if split in dataset.splits[i]]

                    row = [f"{group} - {split} ({kind} {len(inds)})"]
                    for key in suffix_map:
                        cms = [results[i][key] for i in inds if i < len(results)]
                        if len(cms) == 0:
                            vals = [np.nan, np.nan, np.nan]
                        elif kind == "gross":
                            cm = sum(cms)
                            vals = [cm.sensitivity, cm.precision, cm.f1]
                        elif kind == "mean":
                            vals = [
                                np.nanmean([cm.sensitivity for cm in cms]),
                                np.nanmean([cm.precision for cm in cms]),
                                np.nanmean([cm.f1 for cm in cms]),
                            ]
                        row.extend(vals)

                    csvwriter.writerow(row)


def quality_to_csv(csv_filename, dataset, results):
    with open(csv_filename, "w", newline="") as csvfile:
        csvwriter = csv.writer(csvfile, delimiter=",")

        col_names = ["Index", "Split(s)", "Name"]
        col_names.extend(["TP", "FP", "FN", "TN", "Acc", "Sen", "Pre", "F1"])

        csvwriter.writerow(col_names)
        for i, cm in enumerate(results):
            row = [i, ";".join(dataset.splits[i]), dataset.names[i]]
            row.extend(
                [
                    cm.tp,
                    cm.fp,
                    cm.fn,
                    cm.tn,
                    cm.accuracy,
                    cm.sensitivity,
                    cm.precision,
                    cm.f1,
                ]
            )
            csvwriter.writerow(row)
