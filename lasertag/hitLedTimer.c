#include "hitLedTimer.h"
#include "buttons.h"
#include "leds.h"
#include "mio.h"
#include "utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// The lockoutTimer is active for 1/2 second once it is started.
// It is used to lock-out the detector once a hit has been detected.
// This ensure that only one hit is detected per 1/2-second interval.

// StateMachine States
enum hitLedTimer_st_t {
  init_st,           // start state
  disabled_st,       // waits here until enabled
  wait_for_start_st, // waits to start
  hit_Detected_st    // counts 0.5s for LED
};
static enum hitLedTimer_st_t hitLedTimer_currentState;
static enum hitLedTimer_st_t hitLedTimer_oldState;

// Variables
static uint16_t lightTimer;
static bool enabled;
static bool start;

// Helper Functions
void printState();

// Calling this starts the timer.
void hitLedTimer_start() { start = true; }

// Returns true if the timer is currently running.
bool hitLedTimer_running() { return start; }

// Standard tick function.
void hitLedTimer_tick() {
  hitLedTimer_oldState = hitLedTimer_currentState; // saves state for debugging
  // Transitions
  switch (hitLedTimer_currentState) {
  case init_st: // INIT
    hitLedTimer_currentState = disabled_st;
    break;
  case disabled_st: // DISABLED
    // check if enabled
    if (enabled) { // moves to wait for run
      hitLedTimer_currentState = wait_for_start_st;
    } else // still disabled, stays
      hitLedTimer_currentState = disabled_st;
    break;

  case wait_for_start_st: // WAIT FOR START
    // check if not enabled
    if (!enabled) // disabled
      hitLedTimer_currentState = disabled_st;
    else if (start) { // running hit LED
      hitLedTimer_currentState = hit_Detected_st;
      hitLedTimer_turnLedOn();
    } else // not running
      hitLedTimer_currentState = wait_for_start_st;
    break;

  case hit_Detected_st:                          // HIT DETECTED
    if (lightTimer < HIT_LED_TIMER_EXPIRE_VALUE) // still counting
      hitLedTimer_currentState = hit_Detected_st;
    else { // count done, truns off LED
      hitLedTimer_currentState = wait_for_start_st;
      start = false; // lowers running flag
      hitLedTimer_turnLedOff();
      lightTimer = HIT_LED_TIMER_INIT_VAL;
    }
    break;

  default: // DEFAULT
    hitLedTimer_currentState = init_st;
    break;
  }

  // State Actions
  switch (hitLedTimer_currentState) {
  case init_st: // INIT
    break;

  case disabled_st: // DISABLED
    break;

  case wait_for_start_st: // WAIT FOR START
    break;

  case hit_Detected_st: // HIT DETECTED
    lightTimer++;
    break;

  default: // DEFAULT
    break;
  }
}

// Need to init things.
void hitLedTimer_init() {
  hitLedTimer_currentState = init_st; // starts values
  enabled = true;
  start = false;
  lightTimer = HIT_LED_TIMER_INIT_VAL;

  leds_init(false); // init outputs
  mio_init(false);
  mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
  mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, HIT_LED_TIMER_PIN_HIGH); // mio pin
  leds_write(HIT_LED_TIMER_LD0_ON);                               // LED
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
  mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, HIT_LED_TIMER_PIN_LOW); // mio pin
  leds_write(HIT_LED_TIMER_LD0_OFF);                             // LED
}

// Disables the hitLedTimer.
void hitLedTimer_disable() { enabled = false; }

// Enables the hitLedTimer.
void hitLedTimer_enable() { enabled = true; }

// Runs a visual test of the hit LED.
// The test continuously blinks the hit-led on and off.
void hitLedTimer_runTest() {
  printf("HitLedTimer Run test: BTN1 to end\n");
  buttons_init(); // init
  hitLedTimer_init();
  hitLedTimer_enable();

  utils_msDelay(HIT_LED_TIMER_MS_DELAY_TEN);
  // while not button 1
  while (!(buttons_read() & BUTTONS_BTN1_MASK)) { // loop until stopped
    hitLedTimer_start();                          // start hit indicator
    while (hitLedTimer_running()) {               // waits until done
      utils_msDelay(1);
    }
    utils_msDelay(HIT_LED_TIMER_RUNTEST_DELAY); // delay on off
  }
}

// Helper Functions
void printState() {
  // debug sm
  switch (hitLedTimer_currentState) {
  case init_st: // INIT
    printf("Init\n");
    break;

  case disabled_st: // DISABLED
    printf("Disabled\n");
    break;

  case wait_for_start_st: // WAIT FOR START
    printf("Wait for Start\n");
    break;

  case hit_Detected_st: // HIT DETECTED
    printf("Hit Detected\n");
    break;

  default: // DEFAULT
    printf("Error - Default\n");
    break;
  }
}