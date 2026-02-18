# Hall Effect

## Components

### Sensor: Allegro A1304 (SOT-23)

| Parameter | Value |
|-----------|-------|
| Type | Linear, ratiometric |
| Output | Analog |
| Supply | 4.5-5.5V (3.3V works with reduced range) |
| Sensitivity | ~5 mV/G |
| Package | SOT-23 |

One sensor per key (36 total), read through 3x CD74HC4051 multiplexers per half.

### Switch: Owlab Ti HE 40g

| Parameter | Value |
|-----------|-------|
| Travel | 3.4mm |
| Magnetic range | 120-750Gs |
| Actuation | Analog (set in firmware) |
| Force | 40g |
| Mounting | MX hotswap compatible |

### ADC

- nRF52840 SAADC, 12-bit (0-4095)
- 1/6 gain, full-scale = 3.6V
- 3x CD74HC4051 MUX per half, 6 channels each = 18 keys

## Design Considerations

- **3.3V supply:** A1304 is spec'd for 4.5-5.5V but operates at 3.3V with reduced output range. Validated during prototyping Stage 1.
- **MUX settling:** CD74HC4051 on-resistance ~70 Ohm vs SAADC input impedance ~400k Ohm. Need to measure actual settling time (Stage 2).
- **Magnetic crosstalk:** At 19.05mm MX pitch, adjacent magnets may interfere. Measured in Stage 2, mitigated with software compensation or mu-metal shielding if needed.
- **Noise floor:** Determines minimum rapid trigger sensitivity (Stage 1).
- **Errata 86:** nRF52840 SAADC offset drift. Run `CALIBRATEOFFSET` periodically.
