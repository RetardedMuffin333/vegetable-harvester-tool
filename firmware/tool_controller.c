#include <stdint.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// Motor and direction definitions
// -----------------------------------------------------------------------------

typedef enum {
    MOTOR_GRIPPER = 0,   // Motor that opens/closes the cage
    MOTOR_VERTICAL = 1,  // Motor that moves the mechanism up/down
    MOTOR_AUX = 2,       // Spare / future motor (currently unused)
    MOTOR_COUNT
} MotorId;

typedef enum {
    MOTOR_DIR_STOP = 0,
    MOTOR_DIR_FORWARD = 1,
    MOTOR_DIR_REVERSE = 2
} MotorDirection;

// Semantic direction mapping — adjust FORWARD/REVERSE if wiring is flipped
#define GRIPPER_DIR_OPEN   MOTOR_DIR_FORWARD
#define GRIPPER_DIR_CLOSE  MOTOR_DIR_REVERSE
#define VERTICAL_DIR_DOWN  MOTOR_DIR_FORWARD
#define VERTICAL_DIR_UP    MOTOR_DIR_REVERSE

// -----------------------------------------------------------------------------
// Hardware abstraction layer (to be implemented on STM32)
// -----------------------------------------------------------------------------

// Initialize clocks, GPIO, timers, UART, etc.
void hw_init(void);

// Drive a motor with a given direction and duty (0–100% PWM).
void hw_motor_set(MotorId id, MotorDirection dir, uint8_t duty_percent);

// Actively brake / stop a motor.
void hw_motor_brake(MotorId id);

// Optional: read encoder position for a given motor (if encoders are used).
uint32_t hw_encoder_get(MotorId id);

// Millisecond delay (blocking).
void hw_delay_ms(uint32_t ms);

// Start signal (button or command from robot).
bool hw_start_signal(void);

// Emergency stop input (active when true).
bool hw_emergency_stop_signal(void);

// Status LED control.
void hw_set_status_led(bool on);

// Send debug / status text (e.g. UART).
void hw_send_status(const char *text);

// -----------------------------------------------------------------------------
// Tool parameters (prototype-friendly, tune later on hardware)
// -----------------------------------------------------------------------------

enum {
    TOOL_OPEN_GRIPPER_TIME_MS   = 800,   // time to fully open gripper
    TOOL_CLOSE_GRIPPER_TIME_MS  = 900,   // time to fully close gripper
    TOOL_MOVE_DOWN_TIME_MS      = 1200,  // time to move down for cut
    TOOL_MOVE_UP_TIME_MS        = 900,   // time to retract
    TOOL_SETTLE_TIME_MS         = 150,   // pause to let mechanics settle
    TOOL_INTER_CYCLE_DELAY_MS   = 250    // delay between cycles
};

enum {
    TOOL_SPEED_GRIPPER  = 60,  // % PWM
    TOOL_SPEED_VERTICAL = 70   // % PWM
};

// -----------------------------------------------------------------------------
// Internal helpers
// -----------------------------------------------------------------------------

// Run one motor for a given time, checking emergency stop regularly.
static void run_motor_for_time(MotorId id,
                               MotorDirection dir,
                               uint8_t duty_percent,
                               uint32_t time_ms)
{
    const uint32_t step_ms = 10; // how often we check E-stop
    uint32_t elapsed = 0;

    if (duty_percent > 100) {
        duty_percent = 100;
    }

    hw_motor_set(id, dir, duty_percent);

    while (elapsed < time_ms) {
        if (hw_emergency_stop_signal()) {
            // Immediate stop of all motion
            hw_motor_brake(MOTOR_GRIPPER);
            hw_motor_brake(MOTOR_VERTICAL);
            hw_motor_brake(MOTOR_AUX);
            hw_send_status("EMERGENCY STOP\n");
            return;
        }
        hw_delay_ms(step_ms);
        elapsed += step_ms;
    }

    hw_motor_brake(id);
}

// -----------------------------------------------------------------------------
// High-level tool behaviour
// -----------------------------------------------------------------------------

// One harvest cycle:
//
//  1) Open gripper (ensure it’s open)
//  2) Close gripper (grip stem)
//  3) Move down (mechanical cut happens at correct height)
//  4) Move up (retract)
//  5) Open gripper (release cut vegetable)
void tool_run_single_harvest_cycle(void)
{
    hw_send_status("[Tool] Harvest cycle START\n");
    hw_set_status_led(true);

    // 1) Open gripper fully
    hw_send_status("[Tool] Opening gripper\n");
    run_motor_for_time(MOTOR_GRIPPER,
                       GRIPPER_DIR_OPEN,
                       TOOL_SPEED_GRIPPER,
                       TOOL_OPEN_GRIPPER_TIME_MS);
    hw_delay_ms(TOOL_SETTLE_TIME_MS);

    // 2) Close gripper to grip the stem
    hw_send_status("[Tool] Closing gripper\n");
    run_motor_for_time(MOTOR_GRIPPER,
                       GRIPPER_DIR_CLOSE,
                       TOOL_SPEED_GRIPPER,
                       TOOL_CLOSE_GRIPPER_TIME_MS);
    hw_delay_ms(TOOL_SETTLE_TIME_MS);

    // 3) Move down to trigger the passive cutting mechanism
    hw_send_status("[Tool] Moving down for cut\n");
    run_motor_for_time(MOTOR_VERTICAL,
                       VERTICAL_DIR_DOWN,
                       TOOL_SPEED_VERTICAL,
                       TOOL_MOVE_DOWN_TIME_MS);
    hw_delay_ms(TOOL_SETTLE_TIME_MS);

    // 4) Move back up
    hw_send_status("[Tool] Moving up / retract\n");
    run_motor_for_time(MOTOR_VERTICAL,
                       VERTICAL_DIR_UP,
                       TOOL_SPEED_VERTICAL,
                       TOOL_MOVE_UP_TIME_MS);
    hw_delay_ms(TOOL_SETTLE_TIME_MS);

    // 5) Open gripper to release the cut stem
    hw_send_status("[Tool] Releasing gripper\n");
    run_motor_for_time(MOTOR_GRIPPER,
                       GRIPPER_DIR_OPEN,
                       TOOL_SPEED_GRIPPER,
                       TOOL_OPEN_GRIPPER_TIME_MS);

    hw_set_status_led(false);
    hw_send_status("[Tool] Harvest cycle END\n");
}

// Put tool in safe state immediately.
void tool_emergency_stop(void)
{
    hw_motor_brake(MOTOR_GRIPPER);
    hw_motor_brake(MOTOR_VERTICAL);
    hw_motor_brake(MOTOR_AUX);

    hw_set_status_led(false);
    hw_send_status("[Tool] EMERGENCY STOP triggered\n");
}

// -----------------------------------------------------------------------------
// Minimal blocking main loop for prototype testing
// -----------------------------------------------------------------------------

int main(void)
{
    hw_init();

    hw_send_status("[Tool] Vegetable harvester tool controller ready\n");

    bool previous_start_state = false;

    while (1) {
        bool start = hw_start_signal();

        // Emergency stop handling
        if (hw_emergency_stop_signal()) {
            tool_emergency_stop();
            // Stay here until E-stop is released
            while (hw_emergency_stop_signal()) {
                hw_delay_ms(10);
            }
        }

        // Rising edge on start signal triggers one harvest cycle
        if (start && !previous_start_state) {
            tool_run_single_harvest_cycle();
            hw_delay_ms(TOOL_INTER_CYCLE_DELAY_MS);
        }

        previous_start_state = start;
        hw_delay_ms(10);
    }
}
