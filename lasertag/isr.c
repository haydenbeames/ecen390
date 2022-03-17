#include "isr.h"
#include "filter.h"
#include "hitLedTimer.h"
#include "interrupts.h"
#include "lockoutTimer.h"
#include "switches.h"
#include "transmitter.h"
#include "trigger.h"
#include "utils.h"
#include <stdio.h>

// isr provides the isr_function() where you will place functions that require
// accurate timing. A buffer for storing values from the Analog to Digital
// Converter (ADC) is implemented in isr.c Values are added to this buffer by
// the code in isr.c. Values are removed from this queue by code in detector.c

#define ADC_BUFFER_SIZE 100000
#define INIT_VAL 0
#define TEST_SIZE 20
#define RUN_TEST_SIZE 5
#define DECREMENT -1

// This implements a dedicated circular buffer for storing values
// from the ADC until they are read and processed by detector().
// adcBuffer_t is similar to a queue.
typedef struct {
  uint32_t indexIn;               // New values go here.
  uint32_t indexOut;              // Pull old values from here.
  uint32_t elementCount;          // Number of elements in the buffer.
  uint32_t data[ADC_BUFFER_SIZE]; // Values are stored here.
} adcBuffer_t;

// This is the instantiation of adcBuffer.
volatile static adcBuffer_t adcBuffer;
uint32_t incrementIndex(uint32_t currIndex);

// Init adcBuffer.
void adcBufferInit() {
  adcBuffer.indexIn = INIT_VAL;
  adcBuffer.indexOut = INIT_VAL;
  adcBuffer.elementCount = INIT_VAL;
  // initializes buffer with 0's
  for (uint32_t i = INIT_VAL; i < ADC_BUFFER_SIZE; i++) {
    adcBuffer.data[i] = INIT_VAL;
  }
}

// Performs inits for anything in isr.c
void isr_init() {
  adcBufferInit(); // init functions
  trigger_init();
  lockoutTimer_init();
  transmitter_init();
  hitLedTimer_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function() {                              // Task 2
  isr_addDataToAdcBuffer(interrupts_getAdcData()); // adds ADC data to buffer
  trigger_tick();                                  // ticks begin
  transmitter_tick();
  hitLedTimer_tick();
  lockoutTimer_tick();
}

// This adds data to the ADC queue. Data are removed from this queue and used by
// the detector.
void isr_addDataToAdcBuffer(uint32_t adcData) {
  adcBuffer.data[adcBuffer.indexIn] = adcData;
  adcBuffer.indexIn = incrementIndex(adcBuffer.indexIn);
  // buffer full, overwrites to push on new value
  if (adcBuffer.elementCount >= (ADC_BUFFER_SIZE + DECREMENT))
    adcBuffer.indexOut = incrementIndex(adcBuffer.indexOut);
  else {
    ++(adcBuffer.elementCount);
  }
}

// This removes a value from the ADC buffer.
uint32_t isr_removeDataFromAdcBuffer() {
  uint32_t currentIndex =
      adcBuffer.indexOut; // saves current index for data reading
  adcBuffer.indexOut = incrementIndex(adcBuffer.indexOut);
  // lowers elementCount when read
  if (adcBuffer.elementCount >= 1) {
    --adcBuffer.elementCount;
  }
  return adcBuffer.data[currentIndex];
}

// This returns the number of values in the ADC buffer.
uint32_t isr_adcBufferElementCount() { return adcBuffer.elementCount; }

// handles wrapping for indexes. Assumes increment by 1 only
uint32_t incrementIndex(uint32_t currIndex) {
  // controls wrapping around end of buffer
  if (currIndex >= (ADC_BUFFER_SIZE + DECREMENT))
    return INIT_VAL;
  else
    return ++currIndex;
}

// Testing function to make sure ADC Buffer is functional
uint32_t isr_bufferTest() {
  uint32_t myData[] = {
      1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
      13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23}; // test values
  adcBufferInit();
  // runs init
  // adds 20 values to buffer
  for (uint8_t i = 0; i < TEST_SIZE; i++) {
    isr_addDataToAdcBuffer(i);
  }
  for (uint8_t i = 0; i < TEST_SIZE;
       i++) { // loops to remove and read values from buffer
    printf("CURRENT DATA:  %d \n", isr_removeDataFromAdcBuffer());
    printf("INDEX OUT: %d\n", adcBuffer.indexOut);
    printf("\n");
    printf("%d\n", isr_adcBufferElementCount());
  }

  for (uint8_t k = 0; k < RUN_TEST_SIZE; k++) { // prints all values in buffer
    printf("%d ", adcBuffer.data[k]);
  }
  printf("\n");
  printf("%d\n", isr_adcBufferElementCount()); // prints final count in buffer
}
