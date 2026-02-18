# Glossary

| Term | Meaning |
|------|---------|
| ADC | Analog-to-Digital Converter. Converts analog sensor voltage to a digital value. |
| BLE | Bluetooth Low Energy. Wireless protocol used for keyboard-to-dongle communication. |
| BOM | Bill of Materials. List of all parts needed for a build. |
| CAF | Common Application Framework. Event-driven module system from Nordic's nRF Desktop. |
| CRC | Cyclic Redundancy Check. Error-detection code for verifying data integrity. |
| DFU | Device Firmware Update. Flashing new firmware over USB or BLE. |
| DIP | Dual In-line Package. Through-hole component package that fits breadboards. |
| DMA | Direct Memory Access. Hardware moves data to RAM without CPU involvement. |
| DSA | DSA profile keycap. Uniform, non-sculpted keycap shape. |
| DTS | Devicetree Source. Zephyr's hardware description format (pin assignments, peripherals). |
| ESD | Electrostatic Discharge. Protection circuitry against static damage. |
| FSM | Finite State Machine. Code pattern used for rapid trigger logic (IDLE/ACTIVE/RELEASING). |
| GATT | Generic Attribute Profile. BLE protocol layer for sending structured data (HID reports). |
| GPIO | General Purpose Input/Output. Digital pins on the MCU. |
| HID | Human Interface Device. USB/BLE protocol for keyboards, mice, etc. |
| HRM | Home Row Mod. Dual-function keys: tap for letter, hold/deep press for modifier. |
| ISR | Interrupt Service Routine. Code that runs in response to a hardware interrupt. |
| JST | JST connector. Small wire-to-board connector, used here for battery (PH 2.0 size). |
| LiPo | Lithium Polymer battery. Rechargeable battery used in each half. |
| LLPM | Low Latency Packet Mode. Nordic proprietary BLE extension for 1ms connection intervals. |
| MCU | Microcontroller Unit. The chip that runs firmware. Here, the nRF52840. |
| MUX | Multiplexer. Routes multiple sensor inputs to a single ADC channel by switching between them. |
| OTA | Over-The-Air. Wireless firmware updates via BLE. |
| PCB | Printed Circuit Board. |
| PPI | Programmable Peripheral Interconnect. nRF52840 hardware that chains peripherals (timer -> GPIO -> ADC) without CPU. |
| RTT | Real-Time Transfer. SEGGER debug protocol for log output from the MCU. |
| SAADC | Successive Approximation ADC. The specific ADC peripheral on the nRF52840. |
| SMD | Surface Mount Device. Components soldered flat onto a PCB, not through holes. Needs adapters for breadboard use. |
| SOCD | Simultaneous Opposing Cardinal Directions. When left+right or up+down are pressed at the same time. |
| SOIC | Small Outline Integrated Circuit. SMD package type (e.g., SOIC-16 for the MUX). |
| SOT | Small Outline Transistor. SMD package type (e.g., SOT-23 for the A1304 sensor). |
| SPDT | Single Pole Double Throw. Toggle switch type, used here for power on/off. |
| TRRS | Tip-Ring-Ring-Sleeve. 3.5mm audio-style jack used for wired split communication (TX/RX/VCC/GND). |
| UART | Universal Asynchronous Receiver-Transmitter. Serial communication between the two halves over TRRS. |
| 6KRO | 6-Key Rollover. USB HID standard allowing up to 6 simultaneous key presses. |
