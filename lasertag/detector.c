/*
// Student testing functions will return these status values.
typedef uint32_t detector_status_t;   // Used to return status from tests.
#define DETECTOR_STATUS_OK 0          // Everything is A-OK.
#define DETECTOR_STATUS_FAILED_SORT 1 // Something wrong with the sort.
#define DETECTOR_STATUS_FAILED_IGNORE_USER_FREQUENCY                           \
  2 // Didn't properly ignore a specific frequency.
#define DETECTOR_STATUS_FAILED_FIND_MAX_POWER                                  \
  3 // Didn't find the max power frequency number.
#define DETECTOR_STATUS_FAILED_ADC_SCALING                                     \
  4                                      // Didn't properly scale the ADC value.
#define DETECTOR_STATUS_FAILED_GENERAL 5 // Non-specific failure.

typedef uint16_t detector_hitCount_t;

typedef detector_status_t (*sortTestFunctionPtr)(bool, uint32_t, uint32_t,
double[], double[], bool);*/

#include "detector.h"
#include "filter.h"
#include "hitLedTimer.h"
#include "interrupts.h"
#include "isr.h"
#include "lockoutTimer.h"
#include <stdio.h>

// Constants
#define INIT_VAL 0
#define ADC_SCALAR 2047.5
#define ADC_RANGE_ADJUST -1
#define DECREMENT -1
#define FUDGE_FACTOR 1000
#define ADC_MAX 4096.0
#define SCALING_MULTIPLE 2.0
#define SCALING_OFFSET 1.0
#define MEDIAN_INDEX 5
#define BIGGEST_INDEX 9
#define TEST_FACTOR 8
#define F_FACTOR_INDEX 5
#define TEN_CNT_MAX 10
#define INCREMENT 1

static bool hitDetected;
static bool ignoreAllHits;
static uint32_t maxFreq;
static uint16_t detector_hitArray[FILTER_FREQUENCY_COUNT];
static uint32_t fudgeFactorIndex;
static uint8_t sortedIndexArray[FILTER_FREQUENCY_COUNT];
static double sortedPowerValues[FILTER_FREQUENCY_COUNT];
static double thresholdPowerValue;
static double unsortedPowerArray[FILTER_FREQUENCY_COUNT];
static uint16_t ignoredFreq[FILTER_FREQUENCY_COUNT];
static bool runningTests = false;

// state the detector_getHit function
double detector_getHit();

void detector_init(bool ignoredFrequencies[]) {
  hitDetected = false; // sets flags and arrays to zero
  ignoreAllHits = false;
  // initialize arrays to zero and put indices in sortedIndexArray
  for (uint8_t j = 0; j < FILTER_FREQUENCY_COUNT; j++) {
    detector_hitArray[j] = INIT_VAL;
    ignoredFreq[j] = ignoredFrequencies[j];
    sortedIndexArray[j] = j;
  }
  filter_init();
}

// Runs the entire detector: decimating fir-filter, iir-filters,
// power-computation, hit-detection. if interruptsNotEnabled = true, interrupts
// are not running. If interruptsNotEnabled = true you can pop values from the
// ADC queue without disabling interrupts. If interruptsNotEnabled = false, do
// the following:
// 1. disable interrupts.
// 2. pop the value from the ADC queue.
// 3. re-enable interrupts if interruptsNotEnabled was true.
// if ignoreSelf == true, ignore hits that are detected on your frequency.
// Your frequency is simply the frequency indicated by the slide switches
void detector(bool interruptsCurrentlyEnabled) {
  uint32_t elementCount = isr_adcBufferElementCount();
  uint32_t rawAdcValue = INIT_VAL;
  double scaledAdcValue = INIT_VAL;
  static uint8_t runCount = INIT_VAL;
  // iterate through all element counts of circular buffer
  for (uint32_t i = INIT_VAL; i < elementCount; i++) {
    // repeats for all elements
    if (interruptsCurrentlyEnabled) // disables interrupts to safely manipulate
                                    // adcBuffer
      interrupts_disableArmInts();
    rawAdcValue = isr_removeDataFromAdcBuffer(); // pop value
    // check if interrupts are enabled
    if (interruptsCurrentlyEnabled) // reinstates interrupts if going before
      interrupts_enableArmInts();
    // scale the adc value from -1 to 1 from 0-4095
    scaledAdcValue = detector_getScaledAdcValue(rawAdcValue);
    // printf("%d %f\n", rawAdcValue, scaledAdcValue);
    filter_addNewInput(scaledAdcValue); // adds to filter process
    runCount++;                         // increment for another value added
    // reached decimation value, runs all filters and power
    if (runCount >= FILTER_FIR_DECIMATION_FACTOR) {
      runCount = INIT_VAL; // resets for next set of 10
      filter_firFilter();  // FIR filter
      // runs all IIR Filters and Power computations
      for (uint8_t filterNum = INIT_VAL; filterNum < FILTER_FREQUENCY_COUNT;
           filterNum++) {
        filter_iirFilter(filterNum); // IIR
        unsortedPowerArray[filterNum] = filter_computePower(
            filterNum, false, false); // power without force compute or debug
      }
      // Run hit detection
      if (!lockoutTimer_running()) { // no lockoutTimer, not hit yet
        detector_getHit();
      }
    }
  }
}

// Returns true if a hit was detected.
bool detector_hitDetected() { return hitDetected; }

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit() { return maxFreq; }

// Clear the detected hit once you have accounted for it.
void detector_clearHit() {
  hitDetected = false; // lowers flag
}

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) { ignoreAllHits = flagValue; }

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {
  // copies all values from detector_hitArray to passed in hitArray
  for (uint8_t filterNum = INIT_VAL; filterNum < FILTER_FREQUENCY_COUNT;
       filterNum++) {
    hitArray[filterNum] = detector_hitArray[filterNum];
  }
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t index) { fudgeFactorIndex = index; }

// This function sorts the inputs in the unsortedArray and
// copies the sorted results into the sortedArray. It also
// finds the maximum power value and assigns the frequency
// number for that value to the maxPowerFreqNo argument.
// This function also ignores a single frequency as noted below.
// if ignoreFrequency is true, you must ignore any power from frequencyNumber.
// maxPowerFreqNo is the frequency number with the highest value contained in
// the unsortedValues. unsortedValues contains the unsorted values. sortedValues
// contains the sorted values. Note: it is assumed that the size of both of the
// array arguments is 10.
detector_status_t detector_sort(uint32_t *maxPowerFreqNo,
                                double unsortedValues[],
                                double sortedValues[]) {}
// helper function that swaps any array values put into it
void swap(uint8_t swapValues[], uint8_t i, uint8_t j) {
  uint8_t temp = swapValues[i];
  swapValues[i] = swapValues[j];
  swapValues[j] = temp;
}

// insertion sort algorithim helper function
void sort() {
  uint8_t i = 0;
  uint8_t j = 0;
  double temp = 0;
  // populate index array
  for (uint8_t m = 0; m < FILTER_FREQUENCY_COUNT; m++) {
    sortedIndexArray[m] = m;
  }
  // iterate through length of array
  while (i < FILTER_FREQUENCY_COUNT) {
    j = i;
    // compare values in sort
    while ((j > 0) && (unsortedPowerArray[sortedIndexArray[j + DECREMENT]] >
                       unsortedPowerArray[sortedIndexArray[j]])) {
      temp = sortedIndexArray[j + DECREMENT];
      sortedIndexArray[j + DECREMENT] = sortedIndexArray[j];
      sortedIndexArray[j] = temp;
      --j;
    }
    ++i;
  }
}

// helper function that calls detector_sort and checks ignored frequencies to
// return index of highest power
double detector_getHit() {
  // exit if all hits are ignored --> ****invincibility****
  if (ignoreAllHits)
    return 0;
  sort();
  thresholdPowerValue =
      FUDGE_FACTOR * unsortedPowerArray[sortedIndexArray[MEDIAN_INDEX]];
  // check if biggest index is greater than threshold
  if (unsortedPowerArray[sortedIndexArray[BIGGEST_INDEX]] >
          thresholdPowerValue &&
      !ignoredFreq[sortedIndexArray[BIGGEST_INDEX]]) {
    hitDetected = true;
    maxFreq = sortedIndexArray[BIGGEST_INDEX];
    detector_hitArray[maxFreq] += 1; // increases hitCount for the max Freq
  }
  // if there is a hit detected then start hitLedTimer and lockoutTimer
  if (hitDetected) {
    hitLedTimer_start();
    lockoutTimer_start();
  }
  return (double)maxFreq;
}

// Encapsulate ADC scaling for easier testing.
double detector_getScaledAdcValue(isr_AdcValue_t adcValue) {
  return ((double)adcValue / ADC_SCALAR) +
         ADC_RANGE_ADJUST; // divide by half range, minus 1
}

/*******************************************************
 ****************** Test Routines **********************
 ******************************************************/

// Students implement this as part of Milestone 3, Task 3.
void detector_runTest() {
  // Isolated Test
  bool channels[] = {false, false, false, false, false,
                     false, false, false, false, false};
  detector_init(channels);

  // create data set of power values that will show a hit
  unsortedPowerArray[0] = 0;
  unsortedPowerArray[1] = 2.1;
  unsortedPowerArray[2] = 100; // median
  unsortedPowerArray[3] = 78.5;
  unsortedPowerArray[4] = 9.2;
  unsortedPowerArray[5] = 0;
  unsortedPowerArray[6] = 111 * FUDGE_FACTOR + 1; // hit on channel 6
  unsortedPowerArray[7] = 8000;
  unsortedPowerArray[8] = 900;
  unsortedPowerArray[9] = 592;

  detector_getHit();
  printf("%f\n", thresholdPowerValue);
  printf("%f\n", unsortedPowerArray[sortedIndexArray[BIGGEST_INDEX]]);

  // loop starts at highest power, if above threshold and not ignored, then
  // becomes hit if hit detected print maxFreq
  if (detector_hitDetected())
    printf("Hit on channel %d\n", maxFreq);
  else
    printf("No Hit\n");
  detector_clearHit();
  // create data set of power values that won't show power value
  unsortedPowerArray[0] = 100;
  unsortedPowerArray[1] = 2.1;
  unsortedPowerArray[2] = 100;
  unsortedPowerArray[3] = 78.5;
  unsortedPowerArray[4] = 9.2;
  unsortedPowerArray[5] = 0;
  unsortedPowerArray[6] = 11;
  unsortedPowerArray[7] = 82.9; // median
  unsortedPowerArray[8] = 90;
  unsortedPowerArray[9] = 592; // high, but not 1000 * 82.9

  detector_getHit();
  // loop starts at highest power, if above threshold and not ignored, then
  // becomes hit

  // if hit detected print maxFreq
  if (detector_hitDetected())
    printf("Hit on channel %d\n", maxFreq);
  else
    printf("No Hit\n");
}

// Returns 0 if passes, non-zero otherwise.
detector_status_t detector_testAdcScaling() {}