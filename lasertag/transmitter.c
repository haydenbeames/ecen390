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

static uint32_t msCount200 = 0;
volatile static uint16_t timeCount = 0; //counts how long on or off pulse should be based on the frequency
volatile static bool ledFirstPass = false; 
volatile static bool firstPassWaveform = false; 
volatile static bool active = false;
volatile static bool continuous = NON_CONTINUOUS; //non-continuous default
volatile static uint16_t frequencyNumGlobal = 1;
volatile static uint16_t timeCountMax = 5;
volatile static bool transmitterRunningFlag = false;

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
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE); // Write a '0' to JF-1.
}
void transmitter_run() {
    active = true;
    transmitterRunningFlag = true;
}
bool transmitter_running() {
    return transmitterRunningFlag;
}
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
    frequencyNumGlobal = filter_frequencyTickTable[frequencyNumber];
}
uint16_t transmitter_getFrequencyNumber() {
    return frequencyNumGlobal;
}
//disable test mode helper
void transmitter_disableTestMode() {

}
//enable test mode helper
void transmitter_enableTestMode() {
    
}
void transmitter_setContinuousMode(bool continuousModeFlag) {
    continuous = continuousModeFlag;
}
void transmitter_runNoncontinuousTest() {
    transmitter_setContinuousMode(NON_CONTINUOUS);
    //do nonContinuous test while not button pressed
    while (!(buttons_read() & BUTTONS_BTN1_MASK)) { 
      uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;
      transmitter_setFrequencyNumber(switchValue);
      transmitter_run();

      while (transmitter_running()) {  

      }
      utils_msDelay(400);
    }
    transmitter_disableTestMode();
}
void transmitter_runContinuousTest() {  
  uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;
      transmitter_setFrequencyNumber(switchValue);  
    transmitter_setContinuousMode(CONTINUOUS);
     
    //do nonContinuous test while not button pressed
    while (!(buttons_read() & BUTTONS_BTN1_MASK)) { 
      uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;
      transmitter_setFrequencyNumber(switchValue);
    }
}
void transmitter_runTest() {
  printf("starting transmitter_runTest()\n");
  mio_init(false);
  buttons_init();                                         // Using buttons
  switches_init();                                        // and switches.
  transmitter_init();                                     // init the transmitter.
  transmitter_enableTestMode();  

  // Prints diagnostics to stdio.
  while (!(buttons_read() & BUTTONS_BTN1_MASK)) {         // Run continuously until BTN1 is pressed.
    uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
    transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
    transmitter_run();   

    // Start the transmitter.
    while (transmitter_running()) {  
      debugStatePrint();        
      transmitter_tick();
    }
    printf("completed one test period.\n");
  }
  transmitter_disableTestMode();
  
  printf("exiting transmitter_runTest()\n");
}

void transmitter_tick() {  

  //debugStatePrint();
  switch(currentState) {
    case init_st:
        currentState = waiting_for_activation_st;
        break;
      case waiting_for_activation_st:
        //if active go to high state to start waveform
        timeCountMax = (transmitter_getFrequencyNumber() / 2.0);

        //high state if active and continuous
        if ((active == true) && (continuous == NON_CONTINUOUS)) {
            transmitter_set_jf1_to_one();
            currentState = high_st;
        }
        //go to high state if continuous
        else if (continuous == CONTINUOUS) {
            transmitter_set_jf1_to_one();
            currentState = high_st;
        }
        break;
      case high_st:
      //printf("           %d\n", timeCount);
      //printf("%d\n", timeCountMax);
        
        //check if time on is at max
        if (timeCount >= timeCountMax) {
         
            timeCount = 0;
            transmitter_set_jf1_to_zero();    
            currentState = low_st;
        }
        else if (continuous == CONTINUOUS) {
          
            timeCountMax = (transmitter_getFrequencyNumber() / 2.0);
        }
        else { 
             
            currentState = high_st;
        }
        //printf("             %d\n", msCount200);
        break;
      case low_st:
        //printf("%d\n", timeCount);
        //turn JF1 pin off
        if(msCount200 >= TRANSMITTER_PULSE_WIDTH) {
        
            active = false;
            transmitterRunningFlag = false;
            timeCount = 0;
            msCount200 = 0;
            //printf("COMPLETE CYCLE");
            currentState = waiting_for_activation_st;
        }
        //check if time off is at max
        else if ((timeCount >= timeCountMax) && (msCount200 < TRANSMITTER_PULSE_WIDTH)) {
            timeCount = 0;
            transmitter_set_jf1_to_one();
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
        ledFirstPass = false;
        break;
      case high_st:
        timeCount = timeCount + 1;
        msCount200 = msCount200 + 1;
        break;
      case low_st:
        timeCount = timeCount + 1;
        msCount200 = msCount200 + 1;
        break;
     default:
      // print an error message here.
      break;
  }  
}





