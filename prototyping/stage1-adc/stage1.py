#!/usr/bin/env python3
"""Flash the stage1-adc app and capture Stage 1 measurements to CSV.

Needs pyserial; the NCS venv has it:
    ~/.ncs/.venv/bin/python prototyping/stage1-adc/stage1.py flash
    ~/.ncs/.venv/bin/python prototyping/stage1-adc/stage1.py all
README.md next to this file maps each measurement to a Stage 1 row.
"""

import argparse
import csv
import math
import os
import shutil
import statistics
import sys
import time
from dataclasses import dataclass, field

import serial
from serial.tools import list_ports

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
DEFAULT_UF2 = os.path.join(REPO, "build", "stage1-adc", "zephyr", "zephyr.uf2")
DEFAULT_OUT = os.path.join(REPO, "tmp", "stage1-adc")
VOLUME = "/Volumes/NICENANO"
ZEPHYR_USB_VID = 0x2FE3
REST_MS = 1000
SMOOTH_MS = 5
CURVE_SMOOTH_MS = 20
SETTLE_MS = 500
SETTLE_SIGMAS = 10
SETTLE_FLOOR_COUNTS = 10
REPEAT_PRESSES = 20
MIN_TRAVEL_COUNTS = 50


@dataclass
class Row:
    metric: str
    measured: str
    pass_if: str
    fail_if: str
    result: str


@dataclass
class Press:
    start: int
    end: int
    peak: float
    plateau: float


@dataclass
class Trace:
    raw: list
    rest: float
    rest_sigma: float
    sign: int
    depth: list = field(default_factory=list)
    gaps: int = 0

    @classmethod
    def from_raw(cls, raw):
        rest_part = raw[:REST_MS]
        rest = statistics.median(rest_part)
        smooth = moving_average(raw, SMOOTH_MS)
        sign = 1 if max(smooth) - rest >= rest - min(smooth) else -1
        trace = cls(raw, rest, statistics.stdev(rest_part), sign)
        trace.depth = [sign * (x - rest) for x in smooth]
        return trace

    def travel(self):
        return max(self.depth)

    def presses(self):
        full = self.travel()
        if full < MIN_TRAVEL_COUNTS:
            return []
        hi, lo = 0.5 * full, 0.25 * full
        found, start = [], None
        for i, d in enumerate(self.depth):
            if start is None and d >= hi:
                start = i
            elif start is not None and d <= lo:
                found.append(self._press(start, i))
                start = None
        return found

    def _press(self, start, end):
        seg = self.depth[start:end]
        peak = max(seg)
        plateau = statistics.median(d for d in seg if d >= 0.9 * peak)
        return Press(start, end, peak, plateau)


def moving_average(xs, width):
    out, acc = [], 0.0
    for i, x in enumerate(xs):
        acc += x
        if i >= width:
            acc -= xs[i - width]
        out.append(acc / min(i + 1, width))
    return out


def grade(value, pass_ok, fail_bad):
    if pass_ok(value):
        return "PASS"
    if fail_bad(value):
        return "FAIL"
    return "between"


class Board:
    def __init__(self, port):
        self.port = port
        self.serial = serial.Serial(port, 115200, timeout=0.5)
        self.serial.dtr = True
        self.send("")
        time.sleep(0.3)
        self.serial.reset_input_buffer()

    def close(self):
        self.serial.close()

    def send(self, line):
        self.serial.write((line + "\n").encode())
        self.serial.flush()

    def read_line(self, timeout):
        end = time.time() + timeout
        while time.time() < end:
            line = self.serial.readline().decode(errors="replace").strip()
            if line:
                return line
        raise TimeoutError(f"no reply from {self.port} within {timeout}s")

    def expect(self, tag, timeout=5):
        while True:
            line = self.read_line(timeout)
            if line.startswith("ERR,"):
                raise RuntimeError(line)
            if line.startswith(tag + ","):
                return parse_fields(line)

    def info(self):
        self.send("info")
        channels = {}
        while True:
            line = self.read_line(5)
            if line.startswith("CHANNEL,"):
                fields = parse_fields(line)
                channels[fields["name"]] = fields
            elif line.startswith("INFO,"):
                info = parse_fields(line)
                info["channels"] = channels
                return info

    def noise(self, channel):
        self.send(f"noise {channel}")
        return self.expect("NOISE")

    def stream(self, ms, on_sample=None):
        self.send(f"stream {ms}")
        self.expect("STREAM_START")
        samples, next_idx, dropped = [], 0, 0
        while True:
            line = self.read_line(5)
            if line.startswith("S,"):
                _, idx, value = line.split(",")
                idx = int(idx)
                dropped += idx - next_idx
                next_idx = idx + 1
                samples.append((idx, int(value)))
                if on_sample:
                    on_sample(idx)
            elif line.startswith("STREAM_END,"):
                end = parse_fields(line)
                end["dropped"] = str(dropped + int(end["n"]) - next_idx)
                return samples, end
            elif line.startswith("ERR,"):
                raise RuntimeError(line)


def parse_fields(line):
    return dict(part.split("=", 1) for part in line.split(",")[1:] if "=" in part)


def zephyr_ports():
    return [p.device for p in list_ports.comports() if p.vid == ZEPHYR_USB_VID]


def wait_for(pred, timeout, what):
    end = time.time() + timeout
    while time.time() < end:
        result = pred()
        if result:
            return result
        time.sleep(0.25)
    sys.exit(f"timeout waiting for {what}")


def pick_port(requested):
    if requested:
        return requested
    ports = zephyr_ports()
    if len(ports) != 1:
        sys.exit(f"need exactly one Zephyr serial port, found {ports}; pass --port")
    return ports[0]


def open_board(args):
    board = Board(pick_port(args.port))
    info = board.info()
    if info.get("app") != "stage1-adc":
        sys.exit(f"{board.port} does not run stage1-adc ({info}); run the flash command")
    if info["setup_err"] != "0" or info["cal_err"] != "0":
        sys.exit(f"ADC setup failed on the board: {info}")
    return board, info


def flash(args):
    if not os.path.isfile(args.uf2):
        sys.exit(f"{args.uf2} missing; build first (see README.md)")
    if not os.path.isdir(VOLUME):
        port = pick_port(args.port)
        with serial.Serial(port, 115200, timeout=1) as s:
            s.dtr = True
            s.write(b"\nb\n")
            s.flush()
        print(f"sent 'b' to {port}")
        print(f"waiting for {VOLUME} (double-tap RST if it does not appear)")
        wait_for(lambda: os.path.isdir(VOLUME), 60, VOLUME)
        time.sleep(1)
    with open(os.path.join(VOLUME, "INFO_UF2.TXT")) as f:
        uf2_info = f.read()
    if "6.1.1" not in uf2_info:
        sys.exit(f"bootloader is not built for S140 6.1.1:\n{uf2_info}")
    try:
        shutil.copyfile(args.uf2, os.path.join(VOLUME, "zephyr.uf2"))
    except OSError as e:
        print(f"copy ended with {e!r}; expected once the board resets")
    wait_for(lambda: not os.path.isdir(VOLUME), 30, "the bootloader to exit")
    wait_for(zephyr_ports, 30, "the app's serial port")
    time.sleep(1)
    board, info = open_board(args)
    board.close()
    print(f"flashed {args.uf2}; {board.port} reports {info}")


def prompt(args, text):
    print(f"\n{text}")
    if not args.yes:
        input("Press Enter to start. ")


def write_csv(args, name, info, header, rows):
    os.makedirs(args.out, exist_ok=True)
    path = os.path.join(args.out, f"{name}-{time.strftime('%Y%m%d-%H%M%S')}.csv")
    with open(path, "w", newline="") as f:
        f.write(f"# {name} {info}\n")
        w = csv.writer(f)
        w.writerow(header)
        w.writerows(rows)
    print(f"raw: {path}")


def measure_noise(board, info, args):
    prompt(args, "NOISE: hands off the key for the whole capture.")
    rows = []
    for channel in ("ain0", "vdd"):
        for r in range(1, args.rounds + 1):
            n = board.noise(channel)
            rows.append([r, channel, n["n"], n["mean"], n["sigma"], n["min"], n["max"],
                         n["mean_mv"], n["late"]])
            print(f"  {channel} round {r}: mean {n['mean']} sigma {n['sigma']} "
                  f"min {n['min']} max {n['max']} late {n['late']}")
    write_csv(args, "noise", info,
              ["round", "channel", "n", "mean", "sigma", "min", "max", "mean_mv", "late"], rows)

    def med(channel, col):
        return statistics.median(float(r[col]) for r in rows if r[1] == channel)

    sigma = med("ain0", 4)
    pp = max(int(r[6]) - int(r[5]) for r in rows if r[1] == "ain0")
    print(f"  vdd reference: median mean {med('vdd', 3):.1f} counts ({med('vdd', 7):.0f} mV), "
          f"median sigma {med('vdd', 4):.3f}")
    return [
        Row("Rest ADC value", f"{med('ain0', 3):.1f} counts ({med('ain0', 7):.0f} mV)",
            "-", "-", "-"),
        Row("Noise sigma at rest",
            f"{sigma:.2f} counts (median of {args.rounds}, worst p-p {pp})",
            "<5 counts", ">20", grade(sigma, lambda v: v < 5, lambda v: v > 20)),
    ]


def capture_stream(board, info, args, name, seconds, action):
    prompt(args, f"{name.upper()}: hands off for the first second, then {action} "
                 f"Capture lasts {seconds} s.")

    def progress(idx):
        if idx == REST_MS:
            print(f"  GO: {action}")
        if idx % 1000 == 0:
            print(f"  {seconds - idx // 1000} s left", end="\r", flush=True)

    samples, end = board.stream(seconds * 1000, progress)
    gaps = int(end["late"]) + int(end["dropped"])
    print(f"  {len(samples)} samples, late {end['late']}, dropped {end['dropped']}")
    if gaps:
        print("  WARNING: the 1 kHz timeline has gaps; its results are inconclusive")
    write_csv(args, name, info, ["t_ms", "raw"], samples)
    if len(samples) <= REST_MS:
        sys.exit("capture shorter than the hands-off second")
    trace = Trace.from_raw([value for _, value in samples])
    trace.gaps = gaps
    return trace


def gate(trace, rows):
    """Marks graded rows inconclusive when the capture's timeline has gaps."""
    if trace.gaps:
        for row in rows:
            if row.result != "-":
                row.result = "inconclusive"
    return rows


def largest_reversal(depth):
    """Largest drop below the running maximum, with the indices of that maximum and the drop."""
    worst, top, top_at, span = 0.0, -math.inf, 0, (0, 0)
    for i, d in enumerate(depth):
        if d > top:
            top, top_at = d, i
        if top - d > worst:
            worst, span = top - d, (top_at, i)
    return worst, span


def diff_sigma(xs):
    """Noise sigma from successive differences, which a slow drift does not inflate."""
    return statistics.stdev(b - a for a, b in zip(xs, xs[1:])) / math.sqrt(2)


def unsettled_end(head, tail, limit):
    for name, window in (("rest", head), ("bottom-out", tail)):
        spread = max(window) - min(window)
        if spread > limit:
            return f"not settled at {name}: {SETTLE_MS} ms spread {spread} > {limit:.0f} counts"
    return None


def grade_curve(raw):
    """Grades a capture that rests for its first SETTLE_MS and holds bottom-out for its last."""
    head, tail = raw[:SETTLE_MS], raw[-SETTLE_MS:]
    rest, bottom = statistics.median(head), statistics.median(tail)
    sigma = diff_sigma(head)
    limit = 3 * sigma
    settle = max(SETTLE_SIGMAS * sigma, SETTLE_FLOOR_COUNTS)
    sign = 1 if bottom >= rest else -1
    span = abs(bottom - rest)
    print(f"  rest {rest:.1f}, bottom-out {bottom:.1f}, rest sigma {sigma:.2f}, output moves "
          f"{'up' if sign > 0 else 'down'} on press")
    problem = unsettled_end(head, tail, settle)
    if problem is None and span <= settle:
        problem = f"bottom-out reads rest ({span:.0f} counts apart); hold it until the end"
    if problem:
        return [
            Row("Bottom-out ADC value", problem, "-", "-", "-"),
            Row("Usable ADC range", problem, ">500 counts", "<200", "inconclusive"),
            Row("Linearity", problem, "Monotonic", "Dead zones", "inconclusive"),
        ]
    warm = CURVE_SMOOTH_MS - 1
    depth = [sign * (x - rest) for x in moving_average(raw, CURVE_SMOOTH_MS)[warm:]]
    reversal, (top_at, low_at) = largest_reversal(depth)
    where = f" from {top_at + warm} ms to {low_at + warm} ms" if reversal > limit else ""
    return [
        Row("Bottom-out ADC value", f"{bottom:.0f} counts", "-", "-", "-"),
        Row("Usable ADC range", f"{span:.0f} counts", ">500 counts", "<200",
            grade(span, lambda v: v > 500, lambda v: v < 200)),
        Row("Linearity", f"largest reversal {reversal:.1f} counts{where} (3 sigma = "
                         f"{limit:.1f}); dead zones need the CSV plotted",
            "Monotonic", "Dead zones", "FAIL" if reversal > limit else "check plot"),
    ]


def measure_curve(board, info, args):
    t = capture_stream(board, info, args, "curve", args.seconds or 10,
                       "press slowly to bottom-out, reach it at least 1 s before the end, "
                       "and stay there until the capture ends.")
    return gate(t, grade_curve(t.raw))


def measure_taps(board, info, args):
    t = capture_stream(board, info, args, "taps", args.seconds or 20,
                       "tap the key rapidly 50 times.")
    presses = t.presses()
    short = [p for p in presses if p.end - p.start < 5]
    peaks = [p.peak for p in presses]
    gaps = [min(t.depth[a.end:b.start]) for a, b in zip(presses, presses[1:])]
    detail = f"{len(presses)} taps, {len(short)} shorter than 5 ms"
    if peaks:
        detail += f", peak {min(peaks):.0f} to {max(peaks):.0f} counts"
    if gaps:
        detail += f", worst return to rest {int(round(max(gaps)))} counts"
    clean = len(presses) == 50 and not short
    return gate(t, [Row("Rapid tap x50", detail, "50 clean cycles", "missing or extra",
                        "PASS" if clean else "check plot")])


def measure_repeat(board, info, args):
    t = capture_stream(board, info, args, "repeat", args.seconds or 40,
                       f"press to the same depth {REPEAT_PRESSES} times, holding each about "
                       "half a second.")
    plateaus = [p.plateau for p in t.presses()]
    if len(plateaus) != REPEAT_PRESSES:
        return [Row("Repeatability", f"{len(plateaus)} presses found, expected {REPEAT_PRESSES}",
                    "<10 counts", ">50", "inconclusive")]
    spread = max(plateaus) - min(plateaus)
    return gate(t, [Row("Repeatability", f"{spread:.0f} counts across {len(plateaus)} presses",
                        "<10 counts", ">50", grade(spread, lambda v: v < 10, lambda v: v > 50))])


MEASUREMENTS = {
    "noise": measure_noise,
    "curve": measure_curve,
    "taps": measure_taps,
    "repeat": measure_repeat,
}


def print_table(rows):
    print("\n| Metric | Measured | Pass | Fail | Result |")
    print("|---|---|---|---|---|")
    for r in rows:
        print(f"| {r.metric} | {r.measured} | {r.pass_if} | {r.fail_if} | {r.result} |")


def run(args):
    board, info = open_board(args)
    names = list(MEASUREMENTS) if args.command == "all" else [args.command]
    try:
        rows = [row for name in names for row in MEASUREMENTS[name](board, info, args)]
    finally:
        board.close()
    print_table(rows)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("command", choices=["flash", *MEASUREMENTS, "all"])
    ap.add_argument("--port", help="serial port; default: the only Zephyr USB device")
    ap.add_argument("--uf2", default=DEFAULT_UF2, help=f"image to flash (default {DEFAULT_UF2})")
    ap.add_argument("--out", default=DEFAULT_OUT, help=f"CSV directory (default {DEFAULT_OUT})")
    ap.add_argument("--rounds", type=int, default=5, help="noise: runs per channel")
    ap.add_argument("--seconds", type=int, help="stream measurements: capture length")
    ap.add_argument("--yes", action="store_true", help="start each capture without waiting")
    args = ap.parse_args()
    if args.command == "flash":
        flash(args)
    else:
        run(args)


if __name__ == "__main__":
    main()
