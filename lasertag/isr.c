#include "isr.h"
#include "transmitter.h"
#include "trigger.h"
#include "lockoutTimer.h"
#include "hitLedTimer.h"
#include "transmitter.h"
#include "filter.h"
#include "switches.h"

/*
typedef uint32_t
    isr_AdcValue_t; // Used to represent ADC values in the ADC buffer.
*/

// isr provides the isr_function() where you will place functions that require
// accurate timing. A buffer for storing values from the Analog to Digital
// Converter (ADC) is implemented in isr.c Values are added to this buffer by
// the code in isr.c. Values are removed from this queue by code in detector.c

// Performs inits for anything in isr.c
void isr_init() {
    //trigger_init();
    //lockoutTimer_init();
    transmitter_init();
    //hitLedTimer_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function(){ //Task 2
    //trigger_tick();  
    //ticks for all SM's
    //uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
    //printf("SW VAL:  %d\n", switchValue);

    //transmitter_setFrequencyNumber(switchValue);
    transmitter_tick();
    
    
    //hitLedTimer_tick();
    //lockoutTimer_tick();
}

// This adds data to the ADC queue. Data are removed from this queue and used by
// the detector.
void isr_addDataToAdcBuffer(uint32_t adcData);

// This removes a value from the ADC buffer.
uint32_t isr_removeDataFromAdcBuffer();

// This returns the number of values in the ADC buffer.
uint32_t isr_adcBufferElementCount();


