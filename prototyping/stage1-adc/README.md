# Stage 1 ADC instrument

A Zephyr app and a host script that measure the Stage 1 rows in [prototyping/README.md](../README.md#stage-1-single-key-adc). The app is a measurement instrument, not keyboard firmware, so it lives outside `firmware/`.

The app samples AIN0 at 1 kHz with the configuration chosen from the bare-board noise floor result: internal 0.6 V reference, gain 1/6 (0 to 3.6 V full scale), 12-bit, 3 us acquisition, 4x oversampling. It runs SAADC offset calibration once at boot. A second channel reads VDD with the same settings as a reference for comparison. [app.overlay](app.overlay) holds the configuration.

## Wiring

| A1304 pin | SuperMini |
|---|---|
| VCC | 3.3V |
| GND | GND |
| VOUT | AIN0 (P0.02) |

The header comment in `boards/supermini/supermini_nrf52840/supermini_nrf52840_common.dtsi` maps P0.02 to a header pin. Sources disagree on that pin's D number, so confirm P0.02 against your board's silkscreen or pinout card.

## Build and flash

Set up the toolchain as in [docs/setup.md](../../docs/setup.md). From the repo root:

```sh
west build -b supermini_nrf52840/nrf52840/uf2 prototyping/stage1-adc -- -DBOARD_ROOT=$(pwd)
~/.ncs/.venv/bin/python prototyping/stage1-adc/stage1.py flash
```

The build writes `build/stage1-adc/zephyr/zephyr.uf2`. Add `-p always` when `build/` last held another app. `flash` sends `b` to the `--port` you pass, or to the only Zephyr USB serial port, and refuses to guess between several. That reboots this app into the UF2 bootloader, then copies the image and checks that the app answers. A board running other firmware needs a double-tap on RST first.

## Measure

Each command prints what to do, waits for Enter, captures, writes a CSV to `tmp/stage1-adc/` (`--out` changes it), and prints its rows of the Stage 1 table. Every stream capture starts with one second hands-off, which sets the rest value.

| Command | What you do | Stage 1 rows it answers |
|---|---|---|
| `stage1.py noise` | Hands off. Runs 1000 samples 5 times on AIN0, then on VDD | Rest ADC value, Noise sigma at rest |
| `stage1.py curve` | Press slowly to bottom-out, hold, release (10 s) | Bottom-out ADC value, Usable ADC range, Linearity |
| `stage1.py taps` | Tap rapidly 50 times (20 s) | Rapid tap x50 |
| `stage1.py repeat` | Press to the same depth 20 times, holding each (40 s) | Repeatability |
| `stage1.py all` | All four in order, then one combined table | All of the above |

`--seconds` changes a stream's length, `--rounds` the noise runs, and `--yes` skips the Enter prompt. Stream CSVs have one row per received sample (`t_ms,raw`), where `t_ms` is the app's sample index, so a dropped sample leaves a gap in `t_ms` instead of shifting later rows. A `#` line above the header records the app's configuration.

The script detects which way the output moves on press and works in counts away from rest. A press starts at 50% of the largest travel and ends at 25%. Taps shorter than 5 ms count as chatter, and the tap row passes only on exactly 50 taps. Repeatability compares each press's plateau, the median of its samples within 10% of its peak, and is graded only when exactly 20 presses are found; any other count is `inconclusive`, since a press shallower than half the deepest one goes undetected.

The curve takes bottom-out from the hold: the deepest stretch of at least 500 ms that stays within the larger of 6 sigma and 3% of travel. Usable range is that hold's depth. Linearity reports the largest reversal on the approach to the hold, from the last sample under 5% of its depth, against 3 sigma of the rest noise, so an overshoot that falls back to the hold fails. Without a hold, range and linearity read `check plot`. A monotonic approach also reads `check plot`, because dead zones need the travel curve plotted against position, so plot the curve CSV.

Feed the noise sigma and the usable range into the resolution and minimum rapid trigger calculation (issue #17).

## Serial protocol

Commands are lines ending in CR, LF, or CR LF, so `screen /dev/cu.usbmodem* 115200` works by hand. Replies are lines whose first comma-separated field is a tag.

| Command | Reply |
|---|---|
| `info` | One `CHANNEL,name=,resolution=,oversampling=,acq_us=,fullscale_mv=` per channel, then `INFO,app=stage1-adc,rate_hz=,noise_n=,setup_err=,cal_err=` |
| `noise [ain0\|vdd]` | `NOISE,channel=,n=1000,mean=,sigma=,min=,max=,mean_mv=,late=` after 1000 samples at 1 kHz. Default channel `ain0` |
| `stream [ms]` | `STREAM_START,channel=ain0,rate_hz=1000,n=`, then `S,<index>,<raw>` per sample, then `STREAM_END,n=,late=`. No duration streams until the next input byte. A duration that is not a whole number from 1 to 4294967 replies `ERR,bad duration <ms>` |
| `b` | `BOOTLOADER`, then a reboot into the UF2 bootloader |

Any input byte stops a running `noise` or `stream`, and that byte is discarded. An LF right after a CR is part of that CR's terminator, so it neither stops a capture nor counts as a byte. A stopped `noise` replies `ERR,noise interrupted after <n> samples`. Errors and unknown commands reply `ERR,<reason>`. Raw values are 12-bit counts, and sigma uses n - 1.

Samples follow an absolute 1 ms schedule. `late` counts samples that started a full period or more after their slot. The script also counts `S` lines missing from the index sequence. When either count is not zero, it marks that capture's graded rows `inconclusive`.

With nothing on AIN0, the floating pin reads about 440 mV and rises over the first 15 ms of each capture. A driven sensor output should not show that rise, so treat a start-up transient with the sensor wired as a finding.
