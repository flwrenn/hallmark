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
mkdir -p ~/.ncs
python3 -m venv ~/.ncs/.venv
source ~/.ncs/.venv/bin/activate
pip install west
```

## 3. Initialize nRF Connect SDK v3.2.1

```sh
cd ~/.ncs
west init -m https://github.com/nrfconnect/sdk-nrf --mr v3.2.1
west update
```

This clones ~51 repositories. Takes 10-20 minutes depending on network speed.

## 4. Install Python dependencies and register Zephyr

```sh
source ~/.ncs/.venv/bin/activate
west zephyr-export
pip install -r ~/.ncs/zephyr/scripts/requirements.txt
pip install -r ~/.ncs/nrf/scripts/requirements.txt
pip install -r ~/.ncs/bootloader/mcuboot/scripts/requirements.txt
```

## 5. Install Zephyr SDK 0.17.4

Download from [Zephyr SDK releases](https://github.com/zephyrproject-rtos/sdk-ng/releases/tag/v0.17.4).

For macOS ARM64:

```sh
cd ~
curl -LO https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.17.4/zephyr-sdk-0.17.4_macos-aarch64.tar.xz
tar xf zephyr-sdk-0.17.4_macos-aarch64.tar.xz
mv zephyr-sdk-0.17.4 .zephyr-sdk-0.17.4
cd .zephyr-sdk-0.17.4
./setup.sh
```

Verify the ARM toolchain:

```sh
~/.zephyr-sdk-0.17.4/arm-zephyr-eabi/bin/arm-zephyr-eabi-gcc --version
# arm-zephyr-eabi-gcc (Zephyr SDK 0.17.4) 12.2.0
```

## 6. Build the firmware

From the hallmark repo root:

```sh
source ~/.ncs/.venv/bin/activate
export ZEPHYR_BASE=~/.ncs/zephyr

west build -b supermini_nrf52840/nrf52840/uf2 firmware/ -- -DBOARD_ROOT=$(pwd)
```

A successful build produces `build/firmware/zephyr/zephyr.uf2` (~110 KB).

To clean and rebuild:

```sh
rm -rf build
west build -b supermini_nrf52840/nrf52840/uf2 firmware/ -- -DBOARD_ROOT=$(pwd)
```

## 7. Flash via UF2

1. Connect the SuperMini nRF52840 via USB. If macOS asks whether to allow the accessory, click **Allow**, or no drive mounts. The bootloader and the flashed app are separate USB devices, so it may ask again after flashing.
2. Double-tap RST to enter bootloader mode. If the board has no reset button, RST is the third header pin in the column that starts with B+ (B+, GND, RST, VCC). Short it to the GND pin above it twice within half a second. A USB drive named **NICENANO** appears, and the board enumerates as `nice!nano` (USB `239a:00b3`).
3. Check the bootloader:

   ```sh
   cat /Volumes/NICENANO/INFO_UF2.TXT
   ```

   Its `SoftDevice:` line must name S140 6.1.1, whose bootloader loads the app at 0x26000. Bootloader 0.6.0 prints `S140 version 6.1.1`, and 0.6.3 and later print `S140 6.1.1`. A bootloader built with S140 7.x loads it at 0x27000, and this image will not boot there.
4. Copy the UF2 file to the drive:

   ```sh
   cp build/firmware/zephyr/zephyr.uf2 /Volumes/NICENANO/
   ```

   The board resets as soon as it has the whole image, so `cp` can exit with `could not copy extended attributes ... Device not configured` even after a good flash. Check that the board comes back as described below.

After the reset the board enumerates as USB `2fe3:0004` (manufacturer `Zephyr Project`) and the red LED blinks at 1 Hz. The blue LED is the charge indicator, and it flickers when no battery is connected.

## 8. Verify serial output (optional)

The firmware logs over USB CDC ACM serial. After flashing and reset:

```sh
# Find the serial device
ls /dev/cu.usbmodem*

# Connect (replace with actual device)
screen /dev/cu.usbmodem14101 115200
```

Expected output, about 4 seconds after reset. The log thread waits that long on the UF2 variant, so the first lines arrive late but keep the timestamp they were logged at:

```
[00:00:01.409,149] <inf> hallmark: Hallmark firmware v0.0.1
[00:00:01.409,179] <inf> hallmark: Board: SuperMini nRF52840
[00:00:01.409,179] <inf> hallmark: LED blink started (500 ms interval)
[00:00:05.909,912] <inf> hallmark: Heartbeat: 10 toggles
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
| Board | micro-nrf ProMicro nRF52840, nice!nano UF2 bootloader 0.6.0, S140 6.1.1 |

## Board definition

Hallmark builds against its own HWMv2 board definition in `boards/supermini/supermini_nrf52840/`. The `-DBOARD_ROOT=$(pwd)` flag in the build command tells west where to find it.

Zephyr also ships a board for this hardware, [`promicro_nrf52840`](https://docs.zephyrproject.org/latest/boards/others/promicro_nrf52840/doc/index.html), which names the SuperMini in its documentation. The two differ:

- **Link address.** Under the nRF Connect SDK, sysbuild turns on Partition Manager, which ignores the devicetree code partition and links the app at 0x0. Built with NCS v3.2.1, the upstream UF2 variant links at 0x0 too, while the bootloader loads the image at 0x26000. Ours turns Partition Manager off for the UF2 variant in `Kconfig.sysbuild`, and its `CMakeLists.txt` fails the build if Partition Manager comes back on.
- **Peripherals.** Upstream enables UART0, I2C0, I2C1, SPI2, and PWM0, and turns the NFC pins (P0.09, P0.10) into GPIOs. Ours enables UART0 in the base variant only, and P0.09 and P0.10 stay NFC pins.
- **Power.** Ours selects DC/DC mode for REG1 and describes the VCC LDO enable on P0.13.

The board is pin-compatible with Nice!Nano v2. Key differences:
- LED colors are swapped (red = user LED on P0.15, blue = charger)
- P0.13 controls the VCC LDO (active high)
- Uses the Adafruit nRF52 Bootloader with UF2 flashing (flash offset 0x26000)
- Some batches lack the 32.768 kHz crystal. The board selects the crystal as its 32 kHz clock source and waits for it at boot, so for those batches add `-DCONFIG_CLOCK_CONTROL_NRF_K32SRC_RC=y` to the build command.
