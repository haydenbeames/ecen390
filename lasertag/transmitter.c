#include "transmitter.h"
#include <stdbool.h>
#include <stdint.h>
#include "filter.h"
#include "switches.h"
#include "mio.h"

#define TRANSMITTER_OUTPUT_PIN 13 //JF1 pin
#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0


#define INIT_ST_MSG "init_st\n"
#define WAITING_FOR_ACTIVATION_ST_MSG "waiting_for_activation_st\n"
#define HIGH_ST_MSG "high_st\n"
#define LOW_ST_MSG "low_st\n"
#define CONTINUOUS TRUE
#define NON_CONTINUOUS FALSE

static uint16_t msCount200 = 0;
static bool ledFirstPass = 0; 
static bool continuous = NON_CONTINUOUS; //non-continuous default
static bool active


//initialize transmitter
void transmitter_init() {
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);  // Configure the signal direction of the pin to be an output.
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE);
    currentState = init_st;
}

//set JF1 pin to HIGH
void transmitter_set_jf1_to_one() {
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH_VALUE); // Write a '1' to JF-1.
}

//set JF1 pin to LOW
void transmitter_set_jf1_to_zero() {
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE); // Write a '1' to JF-1.
}
void transmitter_run() {
    active = true;
}
bool transmitter_running() {
    if(msCount200) {
        return false;
    }
    else {
        retrurn true;
    }
}
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
    frequency = filter_frequencyTickTable[frequencyNumber];
}
uint16_t transmitter_getFrequencyNumber() {
    return frequencyNumber;
}
void transmitter_runTest() {

}
void transmitter_setContinuousMode(bool continuousModeFlag) {

}
void transmitter_runNoncontinuousTest() {
    continuous = NON_CONTINUOUS;
}
void transmitter_runContinuousTest() {
    continuous = CONTINUOUS;
}
enum transmitter_st_t {
	init_st,                 // Start here, transition out of this state on the first tick.
	waiting_for_activation_st,        // Wait here until the first activation
	high_st,    // set high part of period
	low_st    // set low part of period
};
static enum transmitter_st_t currentState;

//debug helper function
void debugStatePrint() {
  static enum transmitter_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = currentState;     // keep track of the last state that you were in.
    switch(currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf(INIT_ST_MSG);
        break;
      case waiting_for_activation_st:
        printf(WAITING_FOR_ACTIVATION_ST_MSG);
        break;
      case high_st:
        printf(HIGH_ST_MSG);
        break;
      case low_st:
        printf(LOW_ST_MSG);
        break;
     }
  }
}

void transmitter_tick() {  
  // Perform state update first.
  switch(currentState) {
    case init_st:
        transmitter_init();
        currentState = waiting_for_activation_st;
        break;
      case waiting_for_activation_st:
        if(active = true) {
            currentState = high_st;
        }
        break;
      case high_st:
        break;
      case low_st:
        break;
    default:
      // print an error message here.
      break;
  }
  
  // Perform state action next.
  switch(currentState) {
    case init_st:
        break;
      case waiting_for_activation_st:
        break;
      case high_st:
        ++msCount200;
        break;
      case low_st:
        ++msCount200;
        break;
     default:
      // print an error message here.
      break;
  }  
}





