#include "trigger.h"
#include "buttons.h"
#include "mio.h"
#include "transmitter.h"
#include <stdio.h>

// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.

// StateMachine States
enum trigger_st_t {
  init_st,             // start state
  disabled_st,         // waits here until enabled
  wait_For_Trigger_st, // waits until trigger input
  debounce_Trigger_st, // waits for 50ms of trigger on
  transmit_st,         // shoots using transmitter SM
  debounce_Release_st  // waits for 50ms of trigger off
};
static enum trigger_st_t trigger_currentState;
static enum trigger_st_t trigger_oldState;

// State Machine Variables
static bool enabled;
static bool ignoreGunInput;
static bool runTest;
static uint16_t debounceTimer;
static trigger_shotsRemaining_t shotCount;

// Helper Functions
bool triggerPressed(); // returns input from trigger pins
// trigger print state helper function
void triggerprintState(bool runTest); // prints state transitions for debugging

// Init trigger data-structures.
// Determines whether the trigger switch of the gun is connected (see discussion
// in lab web pages). Initializes the mio subsystem.
void trigger_init() {
  trigger_currentState = init_st;
  debounceTimer = TRIGGER_INIT_VAL;
  shotCount = TRIGGER_INIT_VAL;
  runTest = false;
  ignoreGunInput = false; // assumes gun connected, confirmed later

  buttons_init(); // init to read in trigger from BTN0 and MIO pin
  mio_init(false);
  mio_setPinAsInput(TRIGGER_MIO_TRIGGER_PIN); // sets trigger pin as input
  ignoreGunInput = triggerPressed(); // if high already, then gun not connected
}

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable() { enabled = true; }

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() { enabled = false; }

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() { return shotCount; }

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count) {
  shotCount = count;
}

// Standard tick function.
void trigger_tick() {
  trigger_oldState = trigger_currentState; // saves for debugging

  // Transitions
  switch (trigger_currentState) {
  case init_st: // INIT
    trigger_currentState = disabled_st;
    break;

  case disabled_st: // DISABLED
    if (enabled)    // SM enabled, start looking for trigger
      trigger_currentState = wait_For_Trigger_st;
    else // stays disabled
      trigger_currentState = disabled_st;
    break;

  case wait_For_Trigger_st: // WAIT FOR TRIGGER
    if (triggerPressed()) { // trigger pull detected
      trigger_currentState = debounce_Trigger_st;
      debounceTimer = TRIGGER_INIT_VAL; // clears timer
    } else                              // keeps waiting
      trigger_currentState = wait_For_Trigger_st;
    break;

  case debounce_Trigger_st:                           // DEBOUNCE TRIGGER
    if (triggerPressed()) {                           // still pressed
      if (debounceTimer < TRIGGER_DEBOUNCE_TIMER_MAX) // timer not done yet
        trigger_currentState = debounce_Trigger_st;   // stays

      else { // timer done: debounced successfully
        trigger_currentState = transmit_st;
        // printf("D\n");
        transmitter_run(); // starts transmitter - task three
        shotCount--;       // takes away one shot from shotCount
      }
    } else // bounced, not settled yet
      trigger_currentState = wait_For_Trigger_st;
    break;

  case transmit_st:       // TRANSMIT
    if (triggerPressed()) // still pressed
      trigger_currentState = transmit_st;
    else { // ending transmit
      trigger_currentState = debounce_Release_st;
      debounceTimer = TRIGGER_INIT_VAL;
    }
    break;

  case debounce_Release_st:                           // DEBOUNCE RELEASE
    if (triggerPressed()) {                           // bounced
      trigger_currentState = transmit_st;             // returns
    } else {                                          // still low
      if (debounceTimer < TRIGGER_DEBOUNCE_TIMER_MAX) // still checking
        trigger_currentState = debounce_Release_st;
      else { // passed debounce
        trigger_currentState = wait_For_Trigger_st;
        // printf("U\n");
      }
    }
    break;

  default:                          // DEFAULT
    trigger_currentState = init_st; // resets
    break;
  }

  // debug function if transition occurred
  // if (trigger_currentState != trigger_oldState)
  // triggerprintState(runTest);

  // State Actions
  switch (trigger_currentState) {
  case init_st: // INIT
    break;

  case disabled_st: // DISABLED
    break;

  case wait_For_Trigger_st: // WAIT FOR TRIGGER
    break;

  case debounce_Trigger_st: // DEBOUNCE TRIGGER
    debounceTimer++;
    break;

  case transmit_st: // TRANSMIT
    break;

  case debounce_Release_st: // DEBOUNCE RELEASE
    debounceTimer++;
    break;

  default: // DEFAULT
    break;
  }
}

// Runs the test continuously until BTN1 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
void trigger_runTest() {
  printf("Trigger Run Test. Press BTN1 to stop and BTN0 to fire\n");
  trigger_init();   // inits
  runTest = true;   // sets flag for changed debugging print
  trigger_enable(); // starts SM
  trigger_tick();
  while (!(buttons_read() & BUTTONS_BTN1_MASK)) { // runs until BTN1
    trigger_tick();
  }
  trigger_disable(); // stops SM, ends test
  printf("End Trigger Run Test\n");
}

// returns input from trigger pins
bool triggerPressed() {
  return ((!ignoreGunInput &&
           (mio_readPin(TRIGGER_MIO_TRIGGER_PIN) == TRIGGER_HIGH)) ||
          (buttons_read() & BUTTONS_BTN0_MASK));
}

// prints state transitions for debugging - if input is true, does run test
// printing
void triggerprintState(bool runTest) {
  // do prints if run test
  if (!runTest) { // regular debugging prints
    switch (trigger_currentState) {
    case init_st: // INIT
      printf("Init\n");
      break;

    case disabled_st: // DISABLED
      printf("Disabled\n");
      break;

    case wait_For_Trigger_st: // WAIT FOR TRIGGER
      printf("Wait for Trigger\n");
      break;

    case debounce_Trigger_st: // DEBOUNCE TRIGGER
      printf("Debounce Trigger\n");
      break;

    case transmit_st: // TRANSMIT
      printf("Transmit\n");
      break;

    case debounce_Release_st: // DEBOUNCE RELEASE
      printf("Debounce Release\n");
      break;

    default: // DEFAULT
      printf("Error - Default\n");
      break;
    }
  } else { // running runTest, only want D and U printed
    switch (trigger_currentState) {
    case init_st: // INIT
      break;

    case disabled_st: // DISABLED
      break;

    case wait_For_Trigger_st: // WAIT FOR TRIGGER
      break;

    case debounce_Trigger_st: // DEBOUNCE TRIGGER
      break;

    case transmit_st: // TRANSMIT
      break;

    case debounce_Release_st: // DEBOUNCE RELEASE
      break;

    default: // DEFAULT
      printf("Error - Default\n");
      break;
    }
  }
}