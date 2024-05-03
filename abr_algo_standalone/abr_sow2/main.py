import argparse
import json
import pathlib

import matplotlib.pyplot as plt
import numpy as np
from data import Dataset
from model import Model
from preprocessors import (
    Clip,
    LowpassSubtract,
    Notch,
    QualityEvaluator,
    QualityFilter,
    RpeakDetector,
    RpeakEvaluator,
    logit,
    quality_to_csv,
    results_summary_to_csv,
    results_to_csv,
    sigmoid,
)

from plotting import plot_edge_cases, plot_signal_samples_from_dataset

file_dir = pathlib.Path(__file__).parent

parser = argparse.ArgumentParser(description="ABR R+Q model simulator v2")
parser.add_argument("--waist", action="store_true")
args = parser.parse_args()

garment = "waist" if args.waist else "chest"

# --- general parameters:
fs = 320  # sampling frequency of the input signals
output_delay = 16
dtype = np.float32

# --- preprocessing parameters:
notch = Notch(59, 61, fs=fs)

if args.waist:
    lps = LowpassSubtract(tau=11)
    qf = QualityFilter(fs=fs, mask_ab=(0.1, 0.7), quality_ab=None)
    clip = Clip(-1, 1)
else:
    lps = LowpassSubtract(tau=11)
    qf = QualityFilter(fs=fs, mask_ab=(0.15, 0.9), quality_ab=None)
    clip = Clip(-2, 2)


def preprocess(x):
    x = lps(x)
    # x = notch(x)
    x, q = qf(x)
    x = clip(x)
    return x, q


# --- run with TFLite interpreter
TIT_prefix = "TXTIT" if args.waist else "TIT"
results_dir_name = f"results-{garment}"

dataset_file = file_dir / f"myant-rq-{garment}-nodel.npz"
if args.waist:
    model_file = file_dir / "myant-lmu-rq-pod-waist-sow2v2_keras_model.tflite"
else:
    model_file = file_dir / "myant-lmu-rq-pod-chest-sow2v1_keras_model.tflite"

print(f"Using model file: {model_file}")
print(f"Using dataset file: {dataset_file}")

assert model_file.exists()
assert dataset_file.exists()

model = Model(model_file)
model.print_ranges()

dataset = Dataset.from_npz(dataset_file)

results_dir = file_dir / results_dir_name
results_dir.mkdir(exist_ok=True)

if 0:
    # reduce number of signals for quick testing
    dataset = dataset.filter_by_name(lambda name: name.startswith(TIT_prefix))
    # print(dataset.names)

if 0:
    # reduce number of signals for quick testing
    n = 3
    dataset.signals = [dataset.signals[i] for i in range(n)]

    nt = 10000
    # nt = None
    if nt is not None:
        dataset.signals = [s[:nt] for s in dataset.signals]
        for i in range(len(dataset)):
            beat_mask = dataset.beats[i] < nt
            dataset.beats[i] = dataset.beats[i][beat_mask]
            dataset.beat_types[i] = dataset.beat_types[i][beat_mask]


results_json_file = results_dir / "results.json"
results_json = dict(
    model_file=str(model_file.relative_to(file_dir)),
    model_file_size=model_file.stat().st_size,
    # model_timestamp=model_file.stat().st_mtime,
    dataset_file=str(dataset_file.relative_to(file_dir)),
    dataset_file_size=dataset_file.stat().st_size,
    # dataset_timestamp=dataset_file.stat().st_mtime,
)
if results_json_file.exists():
    with results_json_file.open("r") as fh:
        results_json2 = json.load(fh)

    assert results_json2 == results_json, f"{results_json}\n{results_json2}"
else:
    with results_json_file.open("w") as fh:
        json.dump(results_json, fh, indent=2)


# --- preprocess and quantize signals
preproc_signals = []
preproc_quality = []
quant_signals = []
for i, signal in enumerate(dataset.signals):
    assert signal.ndim == 2
    print(f"Preprocessing {dataset.names[i]}")

    signal, quality = preprocess(signal)
    quant_signal = model.quantize(signal)

    preproc_signals.append(signal)
    preproc_quality.append(quality)
    quant_signals.append(quant_signal)

# --- create quality threshold plots
threshold_inds = [
    i
    for i in range(len(dataset))
    if dataset.names[i].startswith(TIT_prefix) and "train" in dataset.splits[i]
]
thresholds = np.logspace(np.log10(0.03), 0.5, 50)

threshold_results = []
for th in thresholds:
    print(f"Evaluating Q threshold: {th}")
    qevaluator = QualityEvaluator(threshold=th)
    cms = []
    for i in threshold_inds:
        noisy_mask = dataset.noisy_mask(i)
        cms.append(qevaluator(noisy_mask, preproc_quality[i]))
    threshold_results.append(sum(cms))

# choose quality threshold to maximize average F1 score
q_threshold = thresholds[
    np.argmax([0.5 * (cm.f1 + cm.neg_f1) for cm in threshold_results])
]

# plot quality curves
fig = plt.figure()
ax = fig.add_subplot(111)

ax.plot(
    thresholds,
    [cm.accuracy for cm in threshold_results],
    color="black",
    label="Accuracy",
)
ax.plot(
    thresholds, [0.5 * (cm.f1 + cm.neg_f1) for cm in threshold_results], label="F1 mean"
)
ax.plot(
    thresholds, [cm.neg_sensitivity for cm in threshold_results], label="Clean sens."
)
ax.plot(thresholds, [cm.sensitivity for cm in threshold_results], label="Noisy sens.")
ax.plot(thresholds, [cm.neg_precision for cm in threshold_results], label="Clean pre.")
ax.plot(thresholds, [cm.precision for cm in threshold_results], label="Noisy pre.")
ax.axvline(
    q_threshold,
    color="black",
    linestyle="--",
    alpha=0.5,
    label=f"Best Threshold (q={q_threshold:.2})",
)
ax.set_xscale("log")
ax.set_xlabel("quality threshold")
ax.set_title(f"Quality metrics vs threshold on {garment} {TIT_prefix} training data")
ax.legend()
fig.savefig(results_dir / "quality_curves.pdf")

# --- compute quality metrics using the best threshold
quality_results = []
qevaluator = QualityEvaluator(threshold=q_threshold)
for i, signal in enumerate(dataset.signals):
    noisy_mask = dataset.noisy_mask(i)
    quality_results.append(qevaluator(noisy_mask, preproc_quality[i]))

quality_result = sum(quality_results)
# print(quality_result)
print(
    "Quality stats: "
    + ", ".join(
        "%s=%0.3f" % (key, getattr(quality_result, key))
        for key in ("accuracy", "sensitivity", "precision", "f1")
    )
)

quality_csv_file = results_dir / "quality.csv"
quality_to_csv(quality_csv_file, dataset, quality_results)

# --- run signals through model
outputs_file = results_dir / "outputs.npy"
if outputs_file.exists():
    print(f"Loading output from: {outputs_file}")
    outputs = np.load(outputs_file, allow_pickle=True)

else:
    outputs = []
    for i, signal in enumerate(quant_signals):
        assert signal.ndim == 2
        print(f"Running {dataset.names[i]}")

        output = model(signal)
        outputs.append(output)

    np.save(outputs_file, outputs)

# --- find best threshold
threshold_inds = [
    i
    for i in range(len(dataset))
    if dataset.names[i].startswith(TIT_prefix) and "train" in dataset.splits[i]
]
thresholds = logit(np.linspace(0.01, 0.99, 50))

threshold_results = []
for th in thresholds:
    print(f"Evaluating threshold: {th}")
    detector = RpeakDetector(threshold=th, delay=output_delay, fs=fs)
    evaluator = RpeakEvaluator(detection_period=50)
    cms = []
    for i in threshold_inds:
        output = outputs[i]
        peak = detector(output)
        result = evaluator(dataset.beats[i], peak)
        cms.append(result["total"])
    threshold_results.append(sum(cms))

best_threshold = thresholds[np.argmax([cm.f1 for cm in threshold_results])]
print(
    f"Best threshold: logit: {best_threshold:0.4f}, "
    f"sigmoid: {sigmoid(best_threshold):0.4f}"
)

# plot f1 curve
fig = plt.figure()
ax = fig.add_subplot(111)
ax.plot(thresholds, [cm.precision for cm in threshold_results], label="Precision")
ax.plot(thresholds, [cm.sensitivity for cm in threshold_results], label="Sensitivity")
ax.plot(thresholds, [cm.f1 for cm in threshold_results], color="black", label="F1")
ax.axvline(
    best_threshold,
    color="black",
    linestyle="--",
    alpha=0.5,
    label=f"Best Threshold (logit={best_threshold:.2})",
)
fig.savefig(results_dir / "f1_curve.pdf")

# --- compute results
detector = RpeakDetector(threshold=best_threshold, delay=output_delay, fs=fs)
evaluator = RpeakEvaluator(detection_period=50)

peaks = []
results = []
for i, output in enumerate(outputs):
    # we consider a timepoint noisy if any channel is noisy
    noisy_mask = dataset.noisy_mask(i).any(axis=-1)

    peak = detector(output)
    peaks.append(peak)

    result = evaluator(dataset.beats[i], peak, noisy_mask)
    results.append(result)

# --- write results to CSV file
results_csv_file = results_dir / "results.csv"
results_to_csv(results_csv_file, dataset, results)
print(f"Wrote {results_csv_file}")

summary_csv_file = results_dir / "summary.csv"
results_summary_to_csv(summary_csv_file, dataset, results, is_waist=args.waist)
print(f"Wrote {summary_csv_file}")

# --- plot results
indices = np.linspace(0, len(dataset) - 1, 15).astype(int)
fig = plot_signal_samples_from_dataset(
    dataset,
    indices=indices,
    # signals=preproc_signals,
    signals=quant_signals,
    outputs=outputs,
    detected_beats=peaks,
    quality_outputs=preproc_quality,
    delay=output_delay,
    apply_sigmoid_to_output=True,
)

sample_fig_file = results_dir / "samples.pdf"
fig.savefig(sample_fig_file)

if not args.waist:
    # currently edge cases are only defined for chest subjects
    fig = plot_edge_cases(
        dataset,
        signals=quant_signals,
        outputs=outputs,
        quality_outputs=preproc_quality,
        detected_beats=peaks,
        apply_sigmoid_to_output=True,
        clip=None,
    )
    edge_fig_file = results_dir / "edge_cases.pdf"
    fig.savefig(edge_fig_file)


# plt.show()
