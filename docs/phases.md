# Development Phases

Post-prototyping phases. Prototyping stages are documented in [prototyping/README.md](../prototyping/README.md).

## Phase 1: Wired

Both halves connected via TRRS cable, one half USB to PC.

- PCB design in KiCad, fabrication via JLCPCB
- 3D printed case (prototype enclosure)
- Port firmware from breadboard to final PCB
- UART transport between halves
- USB HID to PC

## Phase 2: Wireless Split

Peripheral half wireless, central half still USB to PC.

- BLE transport replacing UART
- LLPM for low-latency connection
- Power management, battery life optimization

## Phase 3: Full Wireless

Both halves wireless. XIAO nRF52840 dongle to PC.

- Dongle firmware
- Dual LLPM connections
- Wired USB fallback
- OTA firmware updates (DFU over BLE)

## Phase 4: Metal Case

Replace 3D printed case with CNC machined aluminum.

- CAD modeling based on Phase 1 case dimensions
- Antenna window or cutout for BLE signal (aluminum blocks RF)
- Material and finish selection (anodizing, bead blasting)
- CNC fabrication
