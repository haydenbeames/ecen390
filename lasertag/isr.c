#include "isr.h"
#include "interrupts.h"
#include "transmitter.h"
#include "trigger.h"
#include "lockoutTimer.h"
#include "hitLedTimer.h"
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

#define ADC_BUFFER_SIZE 100000
#define INIT_VAL 0



// This implements a dedicated circular buffer for storing values
// from the ADC until they are read and processed by detector().
// adcBuffer_t is similar to a queue.
typedef struct {
    uint32_t indexIn;   // New values go here.
    uint32_t indexOut;  // Pull old values from here.
    uint32_t elementCount; // Number of elements in the buffer.
    uint32_t data[ADC_BUFFER_SIZE];  // Values are stored here.
} adcBuffer_t;

// This is the instantiation of adcBuffer.
volatile static adcBuffer_t adcBuffer;

// Init adcBuffer.
void adcBufferInit() {
    adcBuffer.indexIn = INIT_VAL;
    adcBuffer.indexOut = INIT_VAL;
    adcBuffer.elementCount = INIT_VAL;
    for(uint32_t i = INIT_VAL; i < ADC_BUFFER_SIZE; i++){
        adcBuffer.data[i] = INIT_VAL;
    }
}

// Performs inits for anything in isr.c
void isr_init() {
    adcBufferInit();
    trigger_init();
    lockoutTimer_init();
    transmitter_init();
    hitLedTimer_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function(){ //Task 2
    trigger_tick();  
    transmitter_tick();
    hitLedTimer_tick();
    lockoutTimer_tick();
}

// This adds data to the ADC queue. Data are removed from this queue and used by
// the detector.
void isr_addDataToAdcBuffer(uint32_t adcData){
    if(incrementIndex(adcBuffer.indexIn) == adcBuffer.indexOut) //buffer full, overwrites to push on new value
        adcBuffer.indexOut = incrementIndex(adcBuffer.indexOut);

     //buffer not full yet or indexOut already adjusted
    adcBuffer.data[indexIn] = adcData; //saves new data and moves index up
    adcBuffer.indexIn = incrementIndex;
    
}

// This removes a value from the ADC buffer.
uint32_t isr_removeDataFromAdcBuffer(){

}

// This returns the number of values in the ADC buffer.
uint32_t isr_adcBufferElementCount(){
    uint32_t size = adcBuffer.indexIn - adcBuffer.indexOut; //takes end minus beginning for size
    if(adBuffer.indexOut > adcuffer.indexIn) //index has wrapped around, add size to account for difference
        size += ADC_BUFFER_SIZE;
    return size;
}

//handles wrapping for indexes. Assumes increment by 1 only
uint32_t incrementIndex(uint32_t currIndex){
    if(currIndex >= (ADC_BUFFER_SIZE - 1))
        return INIT_VAL;
    else
        return ++currIndex;
}