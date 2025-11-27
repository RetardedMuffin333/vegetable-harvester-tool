#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// -----------------------------------------------------------------------------
// This file is a *stub* hardware layer for the vegetable harvester tool.
//
// It lets you compile and "run" the controller on a normal PC without any
// STM32/HAL code. All functions just print what they would do.
//
// On the real robot, you will replace the contents of these functions with
// actual GPIO / timer / UART code for the STM32L476 + MP6619 drivers.
// -----------------------------------------------------------------------------

// These enums must match the ones in tool_controller.c
typedef enum {
    MOTOR_GRIPPER = 0,   // Motor that opens/closes the cage
    MOTOR_VERTICAL = 1,  // Motor that moves the mechanism up/down
    MOTOR_AUX = 2,       // Spare / future motor
    MOTOR_COUNT
} MotorId;

typedef enum {
    MOTOR_DIR_STOP = 0,
    MOTOR_DIR_FORWARD = 1,
    MOTOR_DIR_REVERSE = 2
} MotorDirection;

// Simple internal flags for simulation
static bool status_led_on       = false;
static bool emergency_stop_flag = false;
static bool start_signal_state  = true;   // simulate "start" pressed once

// -----------------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------------

void hw_init(void)
{
    // On STM32: init clocks, GPIOs, timers, UART, etc.
    // In stub: just print a message.
    printf("[HW] init()\n");
}

// -----------------------------------------------------------------------------
// Motor control
// -----------------------------------------------------------------------------

void hw_motor_set(MotorId id, MotorDirection dir, uint8_t duty_percent)
{
    const char *motor_name = "UNKNOWN";
    const char *dir_name   = "STOP";

    switch (id) {
        case MOTOR_GRIPPER:  motor_name = "GRIPPER";  break;
        case MOTOR_VERTICAL: motor_name = "VERTICAL"; break;
        case MOTOR_AUX:      motor_name = "AUX";      break;
        default:             motor_name = "INVALID";  break;
    }

    switch (dir) {
        case MOTOR_DIR_FORWARD: dir_name = "FWD";  break;
        case MOTOR_DIR_REVERSE: dir_name = "REV";  break;
        case MOTOR_DIR_STOP:    dir_name = "STOP"; break;
        default:                dir_name = "???";  break;
    }

    if (duty_percent > 100U) {
        duty_percent = 100U;
    }

    printf("[HW] motor %-8s dir=%-4s duty=%3u%%\n",
           motor_name, dir_name, duty_percent);

    // On STM32: here you would set GPIO direction pins and PWM duty.
}

void hw_motor_brake(MotorId id)
{
    const char *motor_name = "UNKNOWN";

    switch (id) {
        case MOTOR_GRIPPER:  motor_name = "GRIPPER";  break;
        case MOTOR_VERTICAL: motor_name = "VERTICAL"; break;
        case MOTOR_AUX:      motor_name = "AUX";      break;
        default:             motor_name = "INVALID";  break;
    }

    printf("[HW] motor %-8s BRAKE\n", motor_name);

    // On STM32: here you would disable EN or apply a braking configuration.
}

// Optional encoder read (not used yet, returns 0)
uint32_t hw_encoder_get(MotorId id)
{
    (void)id;  // unused in stub
    return 0U;
}

// -----------------------------------------------------------------------------
// Timing
// -----------------------------------------------------------------------------

// Very crude blocking delay using CPU time.
// Good enough for simulation; on STM32 you would use HAL_Delay or a timer.
void hw_delay_ms(uint32_t ms)
{
    const clock_t start = clock();
    const double target_seconds = (double)ms / 1000.0;

    for (;;) {
        const clock_t now = clock();
        const double elapsed = (double)(now - start) / (double)CLOCKS_PER_SEC;
        if (elapsed >= target_seconds) {
            break;
        }
    }
}

// -----------------------------------------------------------------------------
// Inputs: start + emergency stop
// -----------------------------------------------------------------------------

bool hw_start_signal(void)
{
    // In real hardware: read a GPIO or command from the main robot.
    // Stub: simulate a single rising edge at program start.
    if (start_signal_state) {
        start_signal_state = false;
        return true;
    }
    return false;
}

bool hw_emergency_stop_signal(void)
{
    // In real hardware: read an E-stop button / safety input.
    // Stub: always false unless you decide to toggle emergency_stop_flag in code.
    return emergency_stop_flag;
}

// -----------------------------------------------------------------------------
// Outputs: LED + debug text
// -----------------------------------------------------------------------------

void hw_set_status_led(bool on)
{
    status_led_on = on;
    printf("[HW] LED %s\n", status_led_on ? "ON" : "OFF");

    // On STM32: drive a GPIO here.
}

void hw_send_status(const char *text)
{
    if (text != NULL) {
        fputs(text, stdout);
    }

    // On STM32: send via UART, CAN, etc.
}
