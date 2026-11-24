# Prototyping Plan

Validate every risky assumption before PCB fabrication. 6 stages.

## Risk Map

| # | Assumption | If Wrong |
|---|-----------|----------|
| 1 | SAADC has enough resolution for rapid trigger | Full redesign |
| 2 | A1304 + Owlab Ti HE produces usable analog curve | Wrong sensor/switch |
| 3 | MUX scanning >=1kHz and clean | Different topology |
| 4 | Rapid trigger works at achievable resolution | Core feature fails |
| 5 | Depth-based HRM feels better than time-based | Key selling point gone |
| 6 | UART over TRRS reliable at speed | Wiring rework |
| 7 | SuperMini has enough analog pins | Must go bare chip |
| 8 | LLPM works on third-party modules | Wireless compromised |
| 9 | Magnetic crosstalk manageable at 19mm | Shielding needed |

**Order:** Sensor + ADC (risks 1-4) gate everything. Magnetic crosstalk tested in Stage 2. LLPM runs in parallel.

## Mitigations

| Risk | Stage | Mitigation |
|------|-------|------------|
| ADC resolution insufficient | S1 | Oversampling 4-8x, higher-sensitivity sensor, external ADC |
| MUX crosstalk | S2 | 100nF input caps, increase settling time |
| Magnetic crosstalk | S2 | Software compensation, mu-metal shielding |
| LLPM on third-party modules | S6 | Fall back to 7.5ms BLE, use Nordic PCA10059 |
| UART reliability | S3 | Drop to 500kbaud, check cable quality |
| Home row mod misfires | S4 | Per-finger threshold tuning, release confirmation |

## Stage 1: Single Key ADC

**Goal:** A1304 + Owlab Ti HE + nRF52840 SAADC -> clean analog curve.

```
[SuperMini] -- 3.3V/GND/AIN0 -- [A1304 on adapter] <- under [Owlab Ti HE]
```

**Firmware:** SAADC channel 0, 12-bit, 1/6 gain, 1kHz sampling. Print ADC values over USB serial. ~50 lines Zephyr.

**Measure:**
1. Rest ADC value (zero-point)
2. Bottom-out ADC value (max-point)
3. Noise floor: 1000 samples at rest -> sigma
4. Slow press curve: linearity check
5. Rapid tap x50: waveform quality
6. Repeatability: same depth 20x

| Metric | Pass | Fail |
|--------|------|------|
| Usable ADC range | >500 counts | <200 |
| Noise sigma at rest | <5 counts | >20 |
| Linearity | Monotonic | Dead zones |
| Repeatability | <10 count variation | >50 |

**If fail:** Different gain, oversampling 4-8x, different sensor (DRV5055).

### Result: SAADC noise floor without a sensor (2026-11-24)

Measured on a bare micro-nrf ProMicro nRF52840, powered from a Mac over USB, in three sessions. Each run took 1000 readings at 12-bit, gain 1/6, with a 1 ms sleep after each read. The table uses the internal reference. Each figure is the median sigma, in counts, of 5 runs; a range spans the medians of the sessions that measured that row.

| Input | Acquisition | 1x | 4x | 16x |
|---|---|---|---|---|
| AIN0 - AIN0, differential (converter alone) | 10 us | 1.51 to 1.59 | 0.88 to 0.89 | 0.53 to 0.55 |
| AIN0 - AIN0, differential (converter alone) | 3 us | 1.57 to 1.60 | 0.89 to 0.91 | 0.53 to 0.54 |
| VDD, single-ended | 10 us | 5.2 to 7.1 | 2.7 to 3.0 | 1.4 to 1.5 |
| VDD, single-ended | 3 us | 4.7 | 2.3 to 2.4 | 1.4 |

Differential counts are twice the size of single-ended counts in volts.

- **Drift within a session.** In both sessions that interleaved 10 us and 3 us runs, VDD 1x sigma at 10 us rose from about 5.4 to 5.7 in rounds 1 and 2 to 6.7 to 7.1 in rounds 3 to 5. The 3 us runs in the same rounds stayed at 4.5 to 5.0. The cause is unknown.
- **Oversampling.** 4x cuts sigma by about 1.7x to 1.8x on the differential input and 1.9x to 2.4x on VDD. 16x helps less than 1/sqrt(N) predicts on the differential input, about 2.9x instead of 4x, as the readings approach the 12-bit quantization floor.
- **Acquisition time.** On the VDD input, 3 us was no noisier than 10 us. The tied-input differential rows compare residual noise only; they do not measure settling of a real signal.
- **Reference.** In session 1, a VDD/4 reference read about 5% less noise than the internal reference once scaled to the same signal swing (4.9 against 5.2 counts at 1x). That compares the supply against a reference derived from the same supply, so it does not settle the reference for the A1304.
- **Offset.** The differential input reads about -7 counts (-12 mV) with no signal. Run offset calibration at boot and after temperature changes.

**Implication.** Single samples sit around the 5-count pass line, and 4x oversampling gives 2.3 to 3.0 counts. With a 3 us acquisition and about 2 us per conversion, 4x takes about 20 us per key, so 18 keys need about 360 us of conversion time. That leaves about 140 us of Stage 2's 500 us budget for MUX settling and scan overhead, neither of which this test timed. With a 10 us acquisition, conversion alone needs about 864 us. At 800 counts per 3.4 mm, 3 sigma at 4x is at most 0.04 mm, so the ADC alone does not limit rapid trigger on the bench.

**Not measured.** A1304 noise, the real rest-to-bottom-out range, and settling through the MUX. Stage 1 with a sensor answers the first two, Stage 2 the third.

## Stage 2: MUX Scan + Magnetic Crosstalk

**Goal:** 6+ keys via MUX, fast and clean. Magnetic crosstalk at 19mm spacing.

```
[SuperMini] -- AIN0 (P0.02) <- MUX output
            -- P0.17/20/22 (pins D2/D3/D4) -> MUX S0/S1/S2 (shared address)
[CD74HC4051M96 on adapter] -- CH0-CH5 <- 6x A1304 under 6x switches
```

Keep P0.13 and P0.15 off the MUX: P0.13 switches the VCC LDO and P0.15 drives the user LED.

Space 2-3 switches at 19.05mm center-to-center for crosstalk test.

| Metric | Pass | Fail |
|--------|------|------|
| 18-key scan time | <500us | >1000us |
| MUX settling | <5us | >20us |
| Electrical crosstalk | <5 counts | >20 |
| Magnetic crosstalk at 19mm | <3 counts | >10 |

**If fail:** PPI for hardware-automated scanning. 100nF on MUX inputs. Software crosstalk compensation.

## Stage 3: UART Over TRRS

**Goal:** Reliable inter-half analog data at 1kHz.

```
[SuperMini #1 - Peripheral] -- TX/RX -- TRRS -- RX/TX -- [SuperMini #2 - Central] -- USB
```

**Firmware:** Peripheral scans 6 keys, packs into 15-byte packet (sync + half id + 6x16bit + CRC-8), sends at 1Mbaud. Central receives, validates, outputs USB HID.

| Metric | Pass | Fail |
|--------|------|------|
| End-to-end latency | <2ms | >5ms |
| Packet loss | <0.01% | >0.1% |

**If fail:** Drop to 500kbaud, check cable quality.

## Stage 4: Rapid Trigger + Home Row Mods

**Goal:** Validate core HE algorithms with real hardware data.

### 4A: Rapid Trigger

State machine: `IDLE -> ACTIVE -> RELEASING -> ACTIVE...`

Triggers on direction reversal past sensitivity threshold. Test at 0.1, 0.2, 0.5mm sensitivity.

**Critical calculation:**
```
Min sensitivity = noise_sigma x 3 (in mm)
Example: sigma=3 counts, 800 counts/3.4mm -> 3sigma = 0.04mm -> min ~0.05mm
```

### 4B: Depth-Based Home Row Mods

```
Released before depth_threshold AND time_threshold -> TAP (letter)
Crosses depth_threshold at any time                -> HOLD (modifier, immediately)
Held past time_threshold without depth             -> HOLD (fallback)
```

Type "the cat sat" x100 sentences. Count false modifier activations. Compare to time-only baseline.

| Metric | Pass | Fail |
|--------|------|------|
| RT min sensitivity | <=0.2mm | >0.5mm |
| RT false triggers | 0/1000 | >1/100 |
| HRM false activations | <1/500 keystrokes | >1/50 |
| HRM activation speed | <50ms | >150ms |

**If fail:** SAADC oversampling, moving average filter, per-finger depth thresholds, release confirmation.

## Stage 5: Pin Audit

**Goal:** SuperMini vs bare chip decision.

**Need per half:** 3 AIN (MUX outputs) + 1 AIN (battery) + 3 GPIO (MUX addr) + 2 GPIO (UART) + USB + reset = 12-13 pins.

SuperMini exposes 18 header GPIO, 3 of them analog (see Result). Map requirements to actual pins.

| Finding | Action |
|---------|--------|
| >=4 AIN + >=6 GPIO free | Use SuperMini |
| Tight on GPIO | Creative pin sharing |
| <3 usable AIN | Bare chip |

### Result

Desk audit, done before the Stage 2 hardware (#21). Sources:

- Header map: the board comment in `supermini_nrf52840_common.dtsi`, which matches ZMK's nice_nano `arduino_pro_micro_pins.dtsi`.
- Analog channels and radio-adjacent pins: nRF52840 Product Specification v1.9, Table 61 (aQFN73 ball assignments).
- What the pads and the battery circuit connect to: two SuperMini teardowns, [sasodoma/nrf52840-promicro](https://github.com/sasodoma/nrf52840-promicro) (KiCad reverse-engineering) and [joric/nrfmicro#80](https://github.com/joric/nrfmicro/issues/80).

**Needs beyond the Stage 5 list.** `docs/architecture.md` and `docs/hall-effect.md` add no pins. Checked:

- MUX enable: each CD74HC4051 drives its own AIN, so each E pin ties to GND. 0 pins.
- LED: the user LED is on-board at P0.15, and the charge LED is wired to the charger. 0 header pins.
- Power switch: a switch in series with B+ needs no sense pin, and no doc asks for one.
- Reset and USB: the RST header pin (P0.18, `gpio-as-nreset`) and the USB-C connector's dedicated D+/D-. 0 GPIO.
- Sensor power, not in the docs: 18 A1304s draw 7.7 mA each (datasheet typical), about 139 mA, or 5 h on the 750 mAh cell. Wireless use will need to switch sensor power. The P0.13 VCC rail can do it with 0 extra pins if sensors and MUXes run from VCC. A separate load switch costs 1 GPIO.

**Pin assignment.** The same on both halves.

| Function | Header | nRF pin | Note |
|---|---|---|---|
| MUX 0 output | D19 | P0.02 / AIN0 | Stage 2 wiring |
| MUX 1 output | D20 | P0.29 / AIN5 | |
| MUX 2 output | D21 | P0.31 / AIN7 | |
| MUX S0, S1, S2 (shared) | D2, D3, D4 | P0.17, P0.20, P0.22 | Stage 2 wiring. No radio restriction |
| UART TX to TRRS | D1 | P0.06 | Board UART0 pinctrl |
| UART RX from TRRS | D0 | P0.08 | Board UART0 pinctrl |
| Battery voltage | none | VDDH, SAADC input VDDHDIV5 | See battery sensing |
| MUX enable (x3) | none | GND | |

Used: 3 AIN and 5 GPIO.

**Spare pins.** None has an analog channel. The PS limits pins near the radio to signals up to 10 kHz.

| Pins | Limit |
|---|---|
| D5 (P0.24), D6 (P1.00), D7 (P0.11) | None |
| D8 (P1.04), D9 (P1.06), D14 (P1.11), D15 (P1.13), D18 (P1.15) | Up to 10 kHz |
| Center pads P1.01, P1.02, P1.07 | Up to 10 kHz. Solder pads, not header pins |
| D10 (P0.09), D16 (P0.10) | NFC pins. GPIO only after `&uicr` sets `nfct-pins-as-gpios`. Up to 10 kHz |

That leaves 8 spare header pins as the board stands, or 13 with the center pads and NFC pins.

**Battery sensing.** The SuperMini has no working battery divider. The nice!nano v1 divided the battery onto P0.04 (AIN2). The SuperMini schematic draws that divider unpopulated, and the teardown found its trace goes to P0.24 (D5), which has no analog input. P0.04 reaches no header pin or pad. The battery is readable only as VDDH through the SAADC's internal VDDHDIV5 input, which is what ZMK's nice!nano v2 target does (`zmk,battery-nrf-vddh`). Confidence: high that no analog pin carries battery voltage. Two teardowns agree, and neither ZMK's header map nor pdcook's SuperMini Arduino variant routes P0.04 anywhere. Not measured on our boards. With USB connected, VDDH follows USB instead of the cell, so the reading is valid only on battery. In Phase 1 that affects only the central half, which runs on USB.

**Conflicts**

1. No analog headroom. The header has exactly 3 AIN pins and all 3 carry MUX outputs. A fourth analog input (a battery divider, a fourth MUX, a thermistor) does not fit. The fallback without a bare chip: tie the three MUX outputs to one AIN and select a MUX with its E pin, using 3 spare GPIO. That frees 2 AIN but serializes all 18 conversions, which must still fit Stage 2's 500us budget.
2. UART0 on P0.06/P0.08 is the console in the base (J-Link) variant (`supermini_nrf52840.dts`). The PS allows only one peripheral per GPIO, so moving the TRRS link to UARTE1 on the same pins does not help while the console holds them. Either move the base variant's console to RTT and give UART0 to the TRRS link, or put one UART on another pin pair from the unrestricted spares (D5, D6, D7). The 10 kHz pins are too slow for a UART while the radio runs. The UF2 variant logs over USB CDC ACM and leaves UART0 off.
3. D10 and D16 are NFC pins until `nfct-pins-as-gpios` is set.
4. The board owns P0.13 (VCC rail) and P0.15 (LED). Neither is a header pin.
5. ZMK's `D18/A0`-style names are Arduino labels, not nRF channels. A0 is P1.15 and A6 is P0.22, and neither is analog. Only D19, D20, and D21 carry AIN.
6. D5 (P0.24) carries the divider footprint. The teardowns report it unpopulated. Check a board before relying on D5.

**Verdict.** The SuperMini is enough for one half, and a bare nRF52840 is not needed. Against the finding table it has 3 usable AIN, which is neither >=4 nor <3, and 13 free GPIO against 5 needed. The fourth AIN in the need list was the battery, and VDDH reads the battery without a pin, so 3 AIN cover the design. There is no analog margin: the verdict holds only while a half needs 3 analog inputs.

**Unconfirmed until Stage 2 (#21)**

- Scan topology: whether 3 parallel MUX outputs with shared address lines scan 18 keys in under 500us. A different topology changes the AIN and GPIO counts.
- AIN5 and AIN7: Stage 2 wires one MUX on AIN0, so the other two channels stay untested until a 3-MUX build.
- Board checks, needing hardware rather than Stage 2: the VDDH battery reading, and that our boards match the teardowns (center pads P1.01/P1.02/P1.07, no P0.04 pad).

## Stage 6: LLPM Wireless (parallel)

**Goal:** Prove 1ms LLPM on SuperMini, not just Nordic dev kits.

```
[SuperMini #1 - Peripheral] <-- LLPM BLE --> [SuperMini #2 - Central] -- USB
```

**Firmware:** From `samples/bluetooth/llpm`. Send 20-byte payload at 1kHz, measure timing.

| Metric | Pass | Fail |
|--------|------|------|
| Connection interval | 1ms | Falls back to 7.5ms |
| Packet delivery | >99.9% | <99% |
| Range | >2m | <1m |

**If fail:** Check `CONFIG_BT_LL_SOFTDEVICE=y`, `CONFIG_BT_CTLR_SDC_LLPM=y`. Fallback: 7.5ms BLE + Nordic PCA10059 dongle.

## Stage Order

```
S1 (1-key) -> S2 (MUX+mag) -> S3 (UART) -> S4 (algos) -> S5 (pins) -> GO
                                S6 (LLPM, parallel) ----------------------+
```

| Stage | Depends On |
|-------|------------|
| 1: Single key ADC | Parts arrive |
| 2: MUX + crosstalk | Stage 1 |
| 3: UART comms | Stage 2 |
| 4: RT + HRM | Stage 1 data + Stage 2 hw |
| 5: Pin audit | Stage 2 |
| 6: LLPM | Just needs 2 SuperMinis |

**All pass -> Phase 1 (PCB design).**
