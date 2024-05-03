import matplotlib.pyplot as mpl_plt
import numpy as np
from preprocessors import sigmoid


def plot_signal_samples_from_dataset(
    dataset, indices=None, outputs=None, signals=None, names=None, delay=16, **kwargs
):
    assert outputs is None or len(outputs) == len(dataset)

    quality_labels = [
        dataset.noisy_mask(i).astype("float32") for i in range(len(dataset))
    ]

    return plot_signal_samples(
        signals=dataset.signals if signals is None else signals,
        indices=indices,
        outputs=outputs,
        beats=dataset.beats,
        beat_types=dataset.beat_types,
        # classes=dataset.label_info.get("classes", None),
        delay=delay,
        names=dataset.names if names is None else names,
        # quality_classes=dataset.label_info.get("quality_classes", None),
        quality_classes=["clean", "noise"],
        quality_labels=quality_labels,
        n_channels=dataset.n_channels,
        **kwargs,
    )


def plot_signal_samples(
    signals,
    indices=None,
    outputs=None,
    beats=None,
    beat_types=None,
    classes=None,
    detected_beats=None,
    delay=0,
    t_range=(5000, 9000),
    input_channel_slice=(None, None),
    output_channel_slice=(None, None),
    names=None,
    quality_outputs=None,
    quality_classes=None,
    quality_labels=None,
    n_channels=None,
    clip=None,
    apply_sigmoid_to_output=False,
    plt=None,
):
    quality_class_color_dict = {
        "noise": "r",
        "clean": "g",
        "normal": "y",
    }

    assert detected_beats is None or outputs is not None
    plt = mpl_plt if plt is None else plt

    indices = np.arange(len(signals)) if indices is None else indices
    in_ch0, in_ch1 = input_channel_slice
    out_ch0, out_ch1 = output_channel_slice
    if in_ch1 is None and n_channels is not None:
        in_ch1 = n_channels  # discard extra input channels (e.g. quality input)

    rows = len(indices)
    fig, axs = plt.subplots(rows, figsize=(16, 4 * rows))
    axs = [axs] if not isinstance(axs, (list, tuple, np.ndarray)) else axs

    if isinstance(t_range, list):
        assert len(t_range) == len(indices)
    elif isinstance(t_range, tuple):
        t_range = [t_range] * len(indices)
    elif t_range is None:
        t_range = [None] * len(indices)

    print(f"Plotting samples (delay={delay})")
    for i, (ind, ax) in enumerate(zip(indices, axs)):
        signal = signals[ind][..., in_ch0:in_ch1].astype(float)
        nt = len(signal)
        t = np.arange(nt)
        output_t = np.arange(-delay, nt - delay)
        output = outputs[ind][..., out_ch0:out_ch1] if outputs is not None else None
        quality_output = quality_outputs[ind] if quality_outputs is not None else None
        quality_label = quality_labels[ind] if quality_labels is not None else None
        beat = beats[ind] if beats is not None else None
        beat_type = beat_types[ind] if beat_types is not None else None
        detected_beat = detected_beats[ind] if detected_beats is not None else None
        name = names[ind] if names is not None else f"Sample {ind}"

        if clip is not None:
            signal = np.clip(signal, *clip)

        if apply_sigmoid_to_output:
            output = sigmoid(output)

        if t_range[i] is None:
            t_start, t_stop = 0, nt
        else:
            t_start, t_stop = t_range[i]
            t_start = max(nt + t_start, 0) if t_start < 0 else t_start
            t_stop = max(nt + t_stop, 0) if t_stop < 0 else t_stop
            assert (
                t_start < t_stop
            ), f"t_range does not define valid range: {t_start}, {t_stop}"

            m = range(t_start, t_stop)
            signal = signal[m]
            t = t[m]
            quality_label = quality_label[m] if quality_label is not None else None
            # quality output is currently not delayed, so use `m`
            quality_output = quality_output[m] if quality_output is not None else None

            om = (output_t >= t_start) & (output_t < t_stop)
            output_t = output_t[om]
            output = output[om] if output is not None else None

        if beat is not None:
            beat_mask = (beat >= t_start) & (beat < t_stop)
            beat = beat[beat_mask]
            if beat_type is not None:
                beat_type = beat_type[beat_mask]

        if detected_beat is not None:
            # detected_beat = detected_beat - delay
            detected_beat_mask = (detected_beat >= t_start) & (detected_beat < t_stop)
            detected_beat = detected_beat[detected_beat_mask]

        smin, smax = signal.min(), signal.max()
        ymin = smin if output is None else smax - 2 * (smax - smin)

        ax.set_title(f"{name}[{t_start}:{t_stop}]")
        ax.plot(t, signal, label="ECG channel")
        ax.set_ylim([ymin, smax])
        if i == rows - 1:
            ax.set_xlabel("timestep")
        ax.set_ylabel("ECG")
        if beat is not None:
            ax.vlines(
                beat,
                smin,
                smax,
                linestyle=":",
                color="black",
                zorder=5,
                label="Labelled beats",
            )

        ax.legend(loc="center left")

        assert not (output is None and quality_output is not None)
        if output is not None:
            if output.ndim == 1:
                output = output.reshape((-1, 1))

            assert output.ndim == 2
            out_dims = output.shape[-1]
            assert classes is None or len(classes) == out_dims

            omin, omax = output.min(), output.max()
            twinax = ax.twinx()
            for d in range(out_dims):
                label = f"Prediction {d if classes is None else classes[d]}"
                twinax.plot(output_t, output[..., d], label=label)
            twinax.set_ylim([omin, omin + 2.25 * (omax - omin)])
            twinax.set_ylabel("prediction")
            if detected_beat is not None:
                twinax.vlines(
                    detected_beat,
                    omin,
                    omax,
                    linestyle=":",
                    color="red",
                    zorder=5,
                    label="Detected beats",
                )

            twinax.legend(loc="center right")

        if quality_label is not None or quality_output is not None:
            assert quality_classes is not None
            n_cls = len(quality_classes)
            quality_class_colors = [
                quality_class_color_dict.get(q, "k") for q in quality_classes
            ]

            q_channels = signal.shape[-1]

            if quality_label is not None:
                assert quality_label.shape[-1] == q_channels

            if quality_output is not None:
                nt = quality_output.shape[0]
                quality_output = quality_output.reshape(nt, q_channels, -1)
                assert quality_output.ndim == 3
                assert n_channels is None or q_channels == n_channels

                if quality_output.shape[-1] == 1 and n_cls == 2:
                    # binary classification output, so make evidence for "0" class
                    # one minus the evidence for "1" class (assumes sigmoid output)
                    quality_output = np.concatenate(
                        [1 - quality_output, quality_output], axis=-1
                    )

                assert quality_output.shape[-1] == n_cls

                # find maximum quality class for each timepoint/channel
                quality_max = quality_output.max(axis=-1)

            # --- plot "predicted quality label" (based on max quality output)
            ax_min, ax_max = twinax.get_ylim()
            r = 0.1 * (ax_max - ax_min)
            ax_min -= 0.5 * r
            for c in range(q_channels):
                for q, qname in enumerate(quality_classes):
                    # prediction (wider band)
                    if quality_output is not None:
                        twinax.fill_between(
                            t,
                            ax_min - r,
                            ax_min,
                            where=quality_output[:, c, q] >= quality_max[:, c],
                            color=quality_class_colors[q],
                        )

                for q, qname in enumerate(quality_classes):
                    # ground truth (narrow band in middle)
                    if quality_label is not None:
                        twinax.fill_between(
                            t,
                            ax_min - 0.67 * r,
                            ax_min - 0.33 * r,
                            where=quality_label[:, c] == q,
                            color=quality_class_colors[q],
                        )

                # black line to separate channels
                twinax.plot(t, ax_min * np.ones_like(t), "k")

                ax_min -= r

            twinax.set_ylim([ax_min, ax_max])

        if beat_type is not None:
            assert beat is not None and len(beat) == len(beat_type)
            for b, bt in zip(beat, beat_type):
                ax.text(b, smax, bt, color="black")

    return fig


def plot_edge_cases(
    dataset, outputs, quality_outputs=None, detected_beats=None, clip=(-5, 5), **kwargs
):
    edge_cases = {
        "low_activations": {
            "TIT01C-ID08-T1": [
                (1250, 1850),
                (6000, 6500),
                (7300, 7800),
                (14700, 15300),
                (16650, 17050),
            ],
            "TIT01C-ID16-T1": [(2300, 2900), (61800, 62350), (66300, 66400)],
        },
        "abnormal_peak_shape": {
            "TIT01C-ID14-T1": [(67900, 68000)],
            "TIT01C-ID17-T1": [],
        },
        "one_noisy_channel": {"TIT01C-ID06-T1": [(30000, 30500)]},
        "two_noisy_channels": {
            "TIT01C-ID07-T2": [
                (10150, 10800),
                (20100, 20700),
                (21000, 21500),
                (22000, 22600),
                (26400, 27050),
                (40450, 40850),
            ],
            "TIT01C-ID08-T2": [
                (2400, 2800),
                (4100, 4300),
                (11300, 12000),
                (12350, 12900),
                (13200, 13800),
                (15150, 15700),
                (23200, 23900),
                (34200, 34500),
            ],  # Low activations
            "TIT01C-ID24-T4": [
                (34350, 34800),
                (35450, 35750),
                (38950, 39200),
                (39650, 40250),
            ],  # Mislabelled?
        },
    }

    ids = []
    ranges = []
    names = list(dataset.names)
    for edge_case_type, edge_case_samples in edge_cases.items():
        for sample_name in edge_case_samples:
            index = names.index(sample_name)
            regions = edge_case_samples[sample_name]
            names[index] = f"{edge_case_type}: {sample_name}"
            for region in regions:
                t_start = region[0]
                while t_start < region[1]:
                    ids.append(index)
                    ranges.append((t_start, t_start + 1000))
                    t_start += 1000

    return plot_signal_samples_from_dataset(
        dataset=dataset,
        indices=ids,
        t_range=ranges,
        names=names,
        outputs=outputs,
        quality_outputs=quality_outputs,
        detected_beats=detected_beats,
        clip=clip,
        **kwargs,
    )
