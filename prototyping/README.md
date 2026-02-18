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
| UART reliability | S3 | Drop to 500kbaud, add CRC-8 |
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

## Stage 2: MUX Scan + Magnetic Crosstalk

**Goal:** 6+ keys via MUX, fast and clean. Magnetic crosstalk at 19mm spacing.

```
[SuperMini] -- AIN0 <- MUX output
            -- P0.13/14/15 -> MUX S0/S1/S2 (shared address)
[CD74HC4051M96 on adapter] -- CH0-CH5 <- 6x A1304 under 6x switches
```

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

**Firmware:** Peripheral scans 6 keys, packs into 14-byte packet (sync + 6x16bit + checksum), sends at 1Mbaud. Central receives, validates, outputs USB HID.

| Metric | Pass | Fail |
|--------|------|------|
| End-to-end latency | <2ms | >5ms |
| Packet loss | <0.01% | >0.1% |

**If fail:** Drop to 500kbaud, add CRC-8, check cable quality.

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

SuperMini exposes 18 GPIO (~6 analog). Map requirements to actual pins.

| Finding | Action |
|---------|--------|
| >=4 AIN + >=6 GPIO free | Use SuperMini |
| Tight on GPIO | Creative pin sharing |
| <3 usable AIN | Bare chip |

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
