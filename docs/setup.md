# Development Environment Setup

Set up the nRF Connect SDK toolchain for building Hallmark firmware on macOS (Apple Silicon).

## Prerequisites

- macOS with Homebrew
- Python 3.10+ (3.14 tested)
- Git, CMake, Ninja
- ~6 GB disk space for the SDK workspace

## 1. Install system dependencies

```sh
brew install dtc
```

## 2. Create Python virtual environment

```sh
mkdir -p ~/ncs
python3 -m venv ~/ncs/.venv
source ~/ncs/.venv/bin/activate
pip install west
```

## 3. Initialize nRF Connect SDK v3.2.1

```sh
cd ~/ncs
west init -m https://github.com/nrfconnect/sdk-nrf --mr v3.2.1
west update
```

This clones ~51 repositories. Takes 10-20 minutes depending on network speed.

## 4. Install Python dependencies and register Zephyr

```sh
source ~/ncs/.venv/bin/activate
west zephyr-export
pip install -r ~/ncs/zephyr/scripts/requirements.txt
pip install -r ~/ncs/nrf/scripts/requirements.txt
pip install -r ~/ncs/bootloader/mcuboot/scripts/requirements.txt
```

## 5. Install Zephyr SDK 0.17.4

Download from [Zephyr SDK releases](https://github.com/zephyrproject-rtos/sdk-ng/releases/tag/v0.17.4).

For macOS ARM64:

```sh
cd ~
curl -LO https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.17.4/zephyr-sdk-0.17.4_macos-aarch64.tar.xz
tar xf zephyr-sdk-0.17.4_macos-aarch64.tar.xz
cd zephyr-sdk-0.17.4
./setup.sh
```

Verify the ARM toolchain:

```sh
~/zephyr-sdk-0.17.4/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc --version
# arm-zephyr-eabi-gcc (Zephyr SDK 0.17.4) 12.2.0
```

## 6. Build the firmware

From the hallmark repo root:

```sh
source ~/ncs/.venv/bin/activate
export ZEPHYR_BASE=~/ncs/zephyr

west build -b supermini_nrf52840/nrf52840/uf2 firmware/ -- -DBOARD_ROOT=$(pwd)
```

A successful build produces `build/firmware/zephyr/zephyr.uf2` (~115 KB).

To clean and rebuild:

```sh
rm -rf build
west build -b supermini_nrf52840/nrf52840/uf2 firmware/ -- -DBOARD_ROOT=$(pwd)
```

## 7. Flash via UF2

1. Connect the SuperMini nRF52840 via USB.
2. Double-tap the RST button to enter bootloader mode. A USB drive named **NICENANO** appears.
3. Copy the UF2 file to the drive:

```sh
cp build/firmware/zephyr/zephyr.uf2 /Volumes/NICENANO/
```

The board auto-resets and runs the firmware. The red LED should blink at 1 Hz.

## 8. Verify serial output (optional)

The firmware logs over USB CDC ACM serial. After flashing and reset:

```sh
# Find the serial device
ls /dev/cu.usbmodem*

# Connect (replace with actual device)
screen /dev/cu.usbmodem14101 115200
```

Expected output after ~1 second:

```
[00:00:01.000,000] <inf> hallmark: Hallmark firmware v0.0.1
[00:00:01.000,000] <inf> hallmark: Board: SuperMini nRF52840
[00:00:01.000,000] <inf> hallmark: LED blink started (500 ms interval)
[00:00:06.000,000] <inf> hallmark: Heartbeat: 10 toggles
```

Press `Ctrl-A` then `Ctrl-\` to exit screen.

## Tested versions

| Tool | Version |
|---|---|
| nRF Connect SDK | v3.2.1 |
| Zephyr | 4.2.99 (ncs-v3.2.1) |
| Zephyr SDK | 0.17.4 |
| ARM GCC | 12.2.0 |
| West | 1.5.0 |
| Python | 3.14.3 |
| dtc | 1.7.2 |
| Host | macOS (Apple Silicon) |

## Board definition

The SuperMini nRF52840 has no upstream Zephyr board definition. A custom HWMv2 board definition lives in `boards/supermini/supermini_nrf52840/` in this repo.

The board is pin-compatible with Nice!Nano v2. Key differences:
- LED colors are swapped (red = user LED on P0.15, blue = charger)
- P0.13 controls the VCC LDO (active high)
- Uses the Adafruit nRF52 Bootloader with UF2 flashing (flash offset 0x26000)
- Some batches lack the 32.768 kHz crystal

The `-DBOARD_ROOT=$(pwd)` flag in the build command tells west where to find this custom board definition.
