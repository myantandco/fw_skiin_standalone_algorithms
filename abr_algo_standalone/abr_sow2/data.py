import zlib

import numpy as np


class Dataset:
    @classmethod
    def from_npz(cls, file_path):
        data = np.load(file_path, allow_pickle=True)
        args = {key: data[key] for key in data}
        return cls(**args)

    def __init__(self, signals, beats, beat_types, names, splits, noise_regions, fs):
        self.signals = signals
        self.fs = fs if fs is not None else 320

        def default(x, gen):
            return x if x is not None else [gen() for _ in self.signals]

        self.beats = default(beats, lambda: np.array([]))
        self.beat_types = default(beat_types, lambda: np.array([]))
        self.names = default(names, lambda: "")
        self.splits = default(splits, lambda: "")
        self.noise_regions = default(noise_regions, lambda: np.array([]))
        for x in (
            self.beats,
            self.beat_types,
            self.names,
            self.splits,
            self.noise_regions,
        ):
            assert len(x) == len(self.signals)

        assert all(s.ndim == 2 for s in self.signals)
        self.n_channels = self.signals[0].shape[-1]

    def __len__(self):
        return len(self.signals)

    def noisy_mask(self, i):
        mask = np.zeros(self.signals[i].shape, dtype=bool)
        for start, stop, channels in self.noise_regions[i]:
            mask[start:stop, channels] = True
        return mask

    def filter_by_name(self, filt):
        inds = [i for i in range(len(self)) if filt(self.names[i])]
        return Dataset(
            signals=[self.signals[i] for i in inds],
            beats=[self.beats[i] for i in inds],
            beat_types=[self.beat_types[i] for i in inds],
            names=[self.names[i] for i in inds],
            splits=[self.splits[i] for i in inds],
            noise_regions=[self.noise_regions[i] for i in inds],
            fs=self.fs,
        )


def pako_inflate(data):
    decompress = zlib.decompressobj(15)
    decompressed_data = decompress.decompress(data)
    decompressed_data += decompress.flush()
    return decompressed_data


def parse_ECG_data(file):
    with open(file, "rb") as f:
        decoded = f.read()

    result = pako_inflate(decoded)
    raw_data = [x for x in result]

    ECG = []
    ECG_Timestamp = []
    length = len(raw_data)
    counter = 0
    i = 0
    while i < length:
        data = (
            (
                int(raw_data[0 + i])
                | (int(raw_data[1 + i]) << 8)
                | (int(raw_data[2 + i]) << 16)
            )
            << 24
        ) >> 24
        i += 3
        counter += 1
        ECG.append(data)
        if counter == 24:
            data_ts = (
                (
                    int(raw_data[0 + i])
                    | (int(raw_data[1 + i]) << 8)
                    | (int(raw_data[2 + i]) << 16)
                )
                << 24
            ) >> 24
            ECG_Timestamp.append(data_ts)
            i += 4
            counter = 0
    return ECG, ECG_Timestamp


# # --- load data
# if 1:
#     with open("dataset.json", "r") as fh:
#         data = json.load(fh)

#     signals = np.array(data["signal"]).reshape((-1, 3200, 3))
#     beats = np.array(data["beats"])

# else:
#     # --- load data from custom source (e.g. the data that Khalid provided)
#     root_dir = pathlib.Path("~/workspace/myant/datasets/KhalidECG").expanduser()

#     n_channels = 2
#     dirs = [root_dir / f"ECG Ch{i}" for i in range(1, n_channels + 1)]
#     assert all(d.exists() for d in dirs)

#     files = [sorted(d.glob("ecg*")) for d in dirs]
#     n_files = len(files[0])
#     assert n_files > 0
#     assert all(len(ff) == n_files for ff in files)

#     signals = []
#     for i in range(n_files):
#         channels = []
#         for ch in range(3):
#             if ch >= len(files):
#                 channels.append(np.zeros_like(channels[0]))
#             else:
#                 ecg, _ = parse_ECG_data(files[ch][i])
#                 channels.append(ecg)

#         signals.append(np.column_stack(channels))

#     # rescale from raw ADC to mV
#     signals = [signal * adc2mv_factor for signal in signals]

#     print(f"Signal shapes: {', '.join(str(signal.shape) for signal in signals)}")
