# Hallmark

Wireless Hall Effect 36-key split keyboard. Analog sensing, rapid trigger, depth-based home row mods, 1ms LLPM wireless.

**Status:** Prototyping (AliExpress parts delivered; waiting on LCSC shipment — Hall sensors, MUX, caps)

## Specs

| | |
|---|---|
| Layout | Chocofi 36-key (5x3+3 per half), MX hotswap |
| Switch | Owlab Ti HE 40g |
| Sensor | Allegro A1304, 1 per key |
| MCU | nRF52840 (SuperMini) |
| MUX | 3x CD74HC4051 per half |
| Wireless | BLE LLPM 1ms via XIAO nRF52840 dongle |
| Battery | 703040 LiPo 750mAh per half |

## Repository Structure

```
hallmark/
├── docs/
│   ├── architecture.md  # System design, components, block diagram
│   ├── phases.md        # Development phases post-prototyping
│   ├── firmware.md      # Technology choices and planned modules
│   ├── hall-effect.md   # Sensor/switch specs, ADC, design considerations
│   ├── components.md    # Bill of materials
│   ├── references.md    # Datasheets and links
│   └── glossary.md      # Acronyms and terms
├── firmware/            # Zephyr application (C)
├── hardware/
│   ├── pcb/             # PCB design (KiCad)
│   └── case/            # Case design files
├── prototyping/         # 6-stage breadboard validation plan
└── images/              # Photos, renders, diagrams
```

## Docs

- [Architecture](docs/architecture.md) -- system design, components, block diagram
- [Phases](docs/phases.md) -- development phases post-prototyping
- [Firmware](docs/firmware.md) -- technology choices and planned modules
- [Hall Effect](docs/hall-effect.md) -- sensor/switch specs, ADC, design considerations
- [Components](docs/components.md) -- bill of materials
- [Prototyping](prototyping/README.md) -- 6-stage validation plan with pass/fail criteria
- [References](docs/references.md) -- datasheets and links
- [Glossary](docs/glossary.md) -- acronyms and terms

See [GitHub Issues](../../issues) for task tracking and progress.
