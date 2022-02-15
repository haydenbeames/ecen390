#include "trigger.h"

// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.

//typedef uint16_t trigger_shotsRemaining_t;

// Init trigger data-structures.
// Determines whether the trigger switch of the gun is connected (see discussion
// in lab web pages). Initializes the mio subsystem.
void trigger_init(){

}

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable(){

}

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable(){

}

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount(){

}

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count){

}

// Standard tick function.
void trigger_tick(){

}

// Runs the test continuously until BTN1 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
void trigger_runTest(){
    
}

