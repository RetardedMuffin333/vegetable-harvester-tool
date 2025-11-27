# Firmware (Prototype)

This folder contains the **prototype firmware** for the vegetable-harvester end-effector tool.  
The goal of this code is to document and simulate the intended behaviour of the tool without requiring access to the real robot or STM32 hardware.

---

## Files

### `tool_controller.c`
High-level state machine controlling the harvester tool:

- Opens gripper  
- Closes gripper  
- Lowers tool to cutting height  
- Raises tool  
- Releases harvested vegetable  
- Monitors emergency-stop  
- Uses abstract `hw_*` hardware calls (no MCU-specific code inside)

The code is fully hardware-agnostic.  
All physical interactions are delegated to the hardware layer.

---

### `hw_stub.c`
A **PC-friendly stub** implementation of the hardware layer.

- Replaces real STM32 GPIO/PWM/UART with `printf()` simulations  
- Allows running the full tool cycle on any PC  
- Useful for validating logic and behaviour  
- No external dependencies — pure C11

Compile & run locally (optional):

```bash
gcc tool_controller.c hw_stub.c -std=c11 -Wall -Wextra -o harvester_test
./harvester_test
