# Firmware (Prototype)

This folder contains prototype control code for the vegetable-harvester tool.

- `tool_controller.c` – high-level state machine for gripping and cutting.
- Hardware-specific functions (`hw_*`) still need to be implemented for the
  STM32L476RG + MP6619 driver board described in `/electronics/`.

This code is not yet tested on the physical robot due to lack of hardware access.
