"""Synthetic-trace checks for stage1.py's grading. No hardware.

    python3 -m unittest discover -s prototyping/stage1-adc
"""

import argparse
import contextlib
import csv
import io
import os
import random
import sys
import tempfile
import types
import unittest

try:
    import serial  # noqa: F401
except ImportError:
    stub = types.ModuleType("serial")
    stub.tools = types.ModuleType("serial.tools")
    stub.tools.list_ports = types.ModuleType("serial.tools.list_ports")
    sys.modules.update({"serial": stub, "serial.tools": stub.tools,
                        "serial.tools.list_ports": stub.tools.list_ports})

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stage1  # noqa: E402


class FakeBoard(stage1.Board):
    """Replays one stream capture as the app's serial lines."""

    def __init__(self, raw, drop=(), late=0):
        self.port = "fake"
        self.lines = [f"STREAM_START,channel=ain0,rate_hz=1000,n={len(raw)}"]
        self.lines += [f"S,{i},{v}" for i, v in enumerate(raw) if i not in drop]
        self.lines.append(f"STREAM_END,n={len(raw)},late={late}")

    def send(self, line):
        pass

    def read_line(self, timeout):
        return self.lines.pop(0)


def flat(value, ms):
    return [value] * ms


def ramp(a, b, ms):
    return [a + (b - a) * i / ms for i in range(ms)]


def noisy(trace, seed=1):
    rng = random.Random(seed)
    return [round(v + rng.gauss(0, 2)) for v in trace]


def presses(depths, rest=1000, hold=500):
    trace = flat(rest, 1000)
    for d in depths:
        trace += ramp(rest, rest + d, 50) + flat(rest + d, hold) + ramp(rest + d, rest, 50)
        trace += flat(rest, 500)
    return trace


def taps(count, rest=1000, depth=700):
    trace = flat(rest, 1000)
    for _ in range(count):
        trace += ramp(rest, rest + depth, 10) + flat(rest + depth, 20)
        trace += ramp(rest + depth, rest, 10) + flat(rest, 60)
    return trace + flat(rest, 500)


class Capture:
    def __init__(self, measure, raw, **board):
        with tempfile.TemporaryDirectory() as out:
            args = argparse.Namespace(yes=True, seconds=None, out=out, rounds=5)
            with contextlib.redirect_stdout(io.StringIO()):
                rows = measure(FakeBoard(raw, **board), {}, args)
            (name,) = os.listdir(out)
            with open(os.path.join(out, name)) as f:
                self.csv = list(csv.reader(line for line in f if not line.startswith("#")))
        self.rows = {r.metric: r for r in rows}

    def result(self, metric):
        return self.rows[metric].result

    def measured(self, metric):
        return self.rows[metric].measured


class Curve(unittest.TestCase):
    def test_overshoot_then_held_endpoint_reports_the_hold_and_fails(self):
        trace = (flat(1000, 1000) + ramp(1000, 1600, 1000) + ramp(1600, 1350, 300)
                 + flat(1350, 1000) + ramp(1350, 1000, 1000) + flat(1000, 500))
        c = Capture(stage1.measure_curve, noisy(trace))
        bottom = int(c.measured("Bottom-out ADC value").split()[0])
        self.assertLessEqual(abs(bottom - 1350), 5)
        self.assertEqual(c.result("Linearity"), "FAIL")
        self.assertNotEqual(c.result("Usable ADC range"), "PASS")

    def test_clean_stroke_grades_range_and_leaves_dead_zones_to_the_plot(self):
        trace = (flat(2000, 1000) + ramp(2000, 1200, 1500) + flat(1200, 1000)
                 + ramp(1200, 2000, 1500) + flat(2000, 500))
        c = Capture(stage1.measure_curve, noisy(trace))
        bottom = int(c.measured("Bottom-out ADC value").split()[0])
        self.assertLessEqual(abs(bottom - 1200), 5)
        self.assertEqual(c.result("Usable ADC range"), "PASS")
        self.assertEqual(c.result("Linearity"), "check plot")

    def test_stroke_without_a_hold_is_not_graded(self):
        trace = flat(1000, 1000) + ramp(1000, 1600, 1500) + ramp(1600, 1000, 1500)
        c = Capture(stage1.measure_curve, noisy(trace + flat(1000, 500)))
        self.assertEqual(c.result("Usable ADC range"), "check plot")
        self.assertEqual(c.result("Linearity"), "check plot")


class Repeat(unittest.TestCase):
    def test_shallow_press_below_detection_makes_the_run_inconclusive(self):
        c = Capture(stage1.measure_repeat, noisy(presses([600] * 19 + [280])))
        self.assertEqual(c.result("Repeatability"), "inconclusive")
        self.assertIn("19", c.measured("Repeatability"))

    def test_twenty_matching_presses_pass(self):
        c = Capture(stage1.measure_repeat, noisy(presses([600] * 20)))
        self.assertEqual(c.result("Repeatability"), "PASS")

    def test_twenty_detected_presses_with_one_shallow_fail(self):
        c = Capture(stage1.measure_repeat, noisy(presses([600] * 19 + [320])))
        self.assertEqual(c.result("Repeatability"), "FAIL")


class Taps(unittest.TestCase):
    def test_fifty_taps_pass(self):
        c = Capture(stage1.measure_taps, noisy(taps(50)))
        self.assertEqual(c.result("Rapid tap x50"), "PASS")

    def test_extra_taps_do_not_pass(self):
        c = Capture(stage1.measure_taps, noisy(taps(51)))
        self.assertNotEqual(c.result("Rapid tap x50"), "PASS")
        self.assertIn("51 taps", c.measured("Rapid tap x50"))


class Timeline(unittest.TestCase):
    def test_csv_keeps_device_indices_across_a_dropped_sample(self):
        c = Capture(stage1.measure_taps, noisy(taps(50)), drop={1500})
        indices = [int(r[0]) for r in c.csv[1:]]
        self.assertEqual(indices[1499:1501], [1499, 1501])

    def test_dropped_sample_makes_a_passing_capture_inconclusive(self):
        c = Capture(stage1.measure_taps, noisy(taps(50)), drop={1500})
        self.assertEqual(c.result("Rapid tap x50"), "inconclusive")

    def test_late_samples_make_a_passing_capture_inconclusive(self):
        c = Capture(stage1.measure_repeat, noisy(presses([600] * 20)), late=3)
        self.assertEqual(c.result("Repeatability"), "inconclusive")


if __name__ == "__main__":
    unittest.main()
