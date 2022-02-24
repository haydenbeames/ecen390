#include "transmitter.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "filter.h"
#include "switches.h"
#include "utils.h"
#include "buttons.h"
#include "mio.h"

#define TRANSMITTER_OUTPUT_PIN 13 //JF1 pin
#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0
#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 10
#define BOUNCE_DELAY 5


#define INIT_ST_MSG "init_st\n"
#define WAITING_FOR_ACTIVATION_ST_MSG "waiting_for_activation_st\n"
#define HIGH_ST_MSG "high_st\n"
#define LOW_ST_MSG "low_st\n"
#define CONTINUOUS TRUE
#define NON_CONTINUOUS FALSE

static uint16_t msCount200 = 0;
static uint16_t timeCount = 0; //counts how long on or off pulse should be based on the frequency
static bool ledFirstPass = false; 
static bool active = false;
static bool continuous = NON_CONTINUOUS; //non-continuous default
static uint16_t frequencyNum = 0;
static uint16_t timeCountMax = 0;
static bool transmitterRunningFlag = false;

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
    transmitterRunningFlag = true;
}
bool transmitter_running() {
    return transmitterRunningFlag;
}
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
    frequencyNum = filter_frequencyTickTable[frequencyNumber];
}
uint16_t transmitter_getFrequencyNumber() {
    return frequencyNum;
}
void transmitter_disableTestMode() {

}
void transmitter_enableTestMode() {
    
}
void transmitter_setContinuousMode(bool continuousModeFlag) {
    continuous = continuousModeFlag;
}
void transmitter_runNoncontinuousTest() {
    continuous = NON_CONTINUOUS;
    transmitter_tick();
    debugStatePrint();
}
void transmitter_runContinuousTest() {
    continuous = CONTINUOUS;
    transmitter_tick();
    debugStatePrint();
}
void transmitter_runTest() {
  printf("starting transmitter_runTest()\n");
  mio_init(false);
  buttons_init();                                         // Using buttons
  switches_init();                                        // and switches.
  transmitter_init();                                     // init the transmitter.
  transmitter_enableTestMode();                           // Prints diagnostics to stdio.
  while (!(buttons_read() & BUTTONS_BTN1_MASK)) {         // Run continuously until BTN1 is pressed.
    uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
    transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
    transmitter_run();                                    // Start the transmitter.
    while (transmitter_running()) {  
      debugStatePrint();        
       // printf("WHILE LOOP\n");             // Keep ticking until it is done.
    //printf("%s\n", currentState);
      transmitter_tick();                                 // tick.
      utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
    }
    printf("completed one test period.\n");
  }
  transmitter_disableTestMode();
  do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
  printf("exiting transmitter_runTest()\n");
}

void transmitter_tick() {  
  // Perform state update first.
  switch(currentState) {
    case init_st:
        currentState = waiting_for_activation_st;
        break;
      case waiting_for_activation_st:
        //if active go to high state to start waveform
        
        if(active = true) {
            timeCountMax = transmitter_getFrequencyNumber() / 2.0;
            currentState = high_st;
        }
        break;
      case high_st:
      printf("%d ", timeCount);
        //turn JF1 pin on
        if(!ledFirstPass) {
            ledFirstPass = true;
            transmitter_set_jf1_to_one();
        }
        //able to detect changes in frequency in continuous mode
        if (continuous == CONTINUOUS) {
            timeCountMax = transmitter_getFrequencyNumber() / 2.0; 
        }
        //check if time on is at max
        if (timeCount >= timeCountMax) {
            printf("\n");
            timeCount = 0;
            currentState = low_st;
        }
        else {
            
            ledFirstPass = 0;
            currentState = high_st;
        }

        break;
      case low_st:
        //turn JF1 pin off
        if(!ledFirstPass) {
            ledFirstPass = true;
            transmitter_set_jf1_to_zero();
        }
        //check if time off is at max
        if ((timeCount >= timeCountMax) && (msCount200 != TRANSMITTER_PULSE_WIDTH)) {
            timeCount = 0;
            currentState = low_st;
        }
        else if(msCount200 >= TRANSMITTER_PULSE_WIDTH) {
            active = false;
            transmitterRunningFlag = false;
            timeCount = 0;
            msCount200 = 0;
            currentState = waiting_for_activation_st;
        }
        else {
            ledFirstPass = 0;
            currentState = high_st;
        }
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
        timeCount = 0;
        ledFirstPass = 0;
        break;
      case high_st:
        ++timeCount;
        msCount200 = msCount200 + 100;
        break;
      case low_st:
        ++timeCount;
        msCount200 = msCount200 + 100;
        break;
     default:
      // print an error message here.
      break;
  }  
}





