# Firmware

## Technology Choices

| Choice | Why |
|--------|-----|
| nRF Connect SDK | Provides Zephyr, BLE stack (SoftDevice Controller with LLPM), USB HID, DFU, and power management out of the box for the nRF52840. |
| Zephyr RTOS | Hardware abstraction via devicetree (no hardcoded pins), built-in drivers for SAADC/GPIO/UART/USB, and the kernel handles threading and power states. |
| CAF (Common Application Framework) | Event-driven module system from Nordic's nRF Desktop reference. Modules communicate via typed events instead of direct function calls -- keeps things decoupled. |
| PPI (Programmable Peripheral Interconnect) | nRF52840 hardware automation chains timer -> GPIO (MUX select) -> SAADC -> DMA -> RAM. CPU sleeps during the entire 18-key scan. |
| C11 | Required by Zephyr. |

## Modules

"Central" = whichever device is connected to the PC and runs the logic. In Phase 1 that's one of the keyboard halves (USB to PC). In Phase 3 it's the XIAO dongle. The module code doesn't change -- it just runs on a different device.

### Implemented (algorithm cores, pure C, no RTOS deps)

| Module | Runs On | Purpose | Source |
|--------|---------|---------|--------|
| Rapid Trigger | Central | Per-key actuation via direction reversal FSM | `rapid_trigger.c` |
| Home Row Mod | Central | Depth + time hybrid tap/hold decision | `home_row_mod.c` |
| SOCD | Central | Last-input-wins opposing key resolution | `socd.c` |

### Planned

| Module | Runs On | Purpose |
|--------|---------|---------|
| HE Scan | Each half | SAADC + MUX scanning, calibration offsets |
| Keymap | Central | Layers, combos, macros |
| Transport | All | UART (Phase 1) or BLE (Phase 2) between halves |
| Calibration | Each half | Boot auto-zero, runtime drift compensation |
| Power Manager | Each half | Adaptive scan rate, deep sleep, battery monitoring |
