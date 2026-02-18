# System Architecture

Hall Effect 36-key split keyboard with analog sensing.

## Target Components

| | |
|---|---|
| Layout | Chocofi 36-key (5x3+3 per half), MX hotswap |
| Switch | Owlab Ti HE 40g (3.4mm travel) |
| Sensor | Allegro A1304 (SOT-23), 1 per key |
| MCU | nRF52840 (SuperMini or bare -- TBD by prototyping Stage 5) |
| MUX | 3x CD74HC4051 per half (18 keys -> 3 ADC channels) |
| Wireless | BLE, dongle = XIAO nRF52840 |

## Block Diagram

```
LEFT HALF                               RIGHT HALF
18x Owlab Ti HE + A1304 + 3x MUX       18x Owlab Ti HE + A1304 + 3x MUX
nRF52840                                nRF52840
         |                                       |
         +------------>  DONGLE (XIAO)  <--------+
                            | USB HID
                           PC
```

Each half scans + calibrates + transmits raw positions. All logic (rapid trigger, HRM, SOCD, keymap, HID) runs on the central device.
