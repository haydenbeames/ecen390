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

typedef detector_status_t (*sortTestFunctionPtr)(bool, uint32_t, uint32_t, double[], double[], bool);*/

#include "filter.h"
#include"lockoutTimer.h"
#include "hitLedTimer.h"
#include "isr.h"
#include "detector.h"
#include "interrupts.h"
#include <stdio.h>


//Constants
#define INIT_VAL 0
#define ADC_SCALAR 2047.5
#define ADC_RANGE_ADJUST -1
#define FUDGE_FACTOR 1000
#define MEDIAN_INDEX 4

static bool hitDetected;
static bool ignoreAllHits;
static uint32_t maxFreq;
static uint16_t detector_hitArray[FILTER_FREQUENCY_COUNT];
static uint32_t fudgeFactorIndex;
static uint8_t sortedIndexArray[FILTER_FREQUENCY_COUNT];
static double sortedPowerValues[FILTER_FREQUENCY_COUNT];
static uint32_t thresholdPowerValue;
static double unsortedPowerArray[FILTER_FREQUENCY_COUNT];
static uint16_t ignoredFreq[FILTER_FREQUENCY_COUNT];
// have to init things.
// bool array is indexed by frequency number, array location set for true to
// ignore, false otherwise. This way you can ignore multiple frequencies.
void detector_init(bool ignoredFrequencies[]){
    hitDetected = false; //sets flags and arrays to zero
    ignoreAllHits = false;
    for(uint8_t j = 0; j < FILTER_FREQUENCY_COUNT; j++) {
        detector_hitArray[j] = INIT_VAL;
        unsortedPowerArray[j] = INIT_VAL;
        ignoredFreq[j] = ignoredFrequencies[j];
        sortedIndexArray[j] = j;
    }
    //filter_init();
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
void detector(bool interruptsCurrentlyEnabled){
    uint32_t elementCount = FILTER_FREQUENCY_COUNT; //isr_adcBufferElementCount();
    uint32_t rawAdcValue = INIT_VAL;
    double scaledAdcValue = INIT_VAL;
    uint8_t runCount = FILTER_FREQUENCY_COUNT;
    
    for(uint32_t i = INIT_VAL; i < elementCount; i++){ //repeats for all elements
        if(interruptsCurrentlyEnabled) //disables interrupts to safely manipulate adcBuffer
            interrupts_disableArmInts();
        
        rawAdcValue = isr_removeDataFromAdcBuffer(); //pop value
        if(interruptsCurrentlyEnabled) //reinstates interrupts if going before
            interrupts_enableArmInts();
        
        //scale the adc value from -1 to 1 from 0-4095
        scaledAdcValue = detector_getScaledAdcValue(rawAdcValue);
        filter_addNewInput(scaledAdcValue); //adds to filter process
        runCount++; //increment for another value added

        //reached decimation value, runs all filters and power
        if(runCount >= FILTER_FIR_DECIMATION_FACTOR){ 
            runCount = INIT_VAL; //resets for next set of 10

            filter_firFilter(); //FIR filter
            //runs all IIR Filters and Power computations
            for(uint8_t filterNum = INIT_VAL; filterNum < FILTER_FREQUENCY_COUNT; filterNum++){
                if(i = 0) {
                    filter_computePower(filterNum, true, false);
                }
                filter_iirFilter(filterNum); //IIR
                filter_computePower(filterNum, false, false); //power without force compute or debug
            }

            //Run hit detection
            if(!lockoutTimer_running()){ //no lockoutTimer, not hit yet
                //hit-detection algorithm
                //filter_getCurrentPowerValues(unsortedPowerArray);
                detector_sort(&maxFreq, unsortedPowerArray, sortedPowerValues); //sorts array
                thresholdPowerValue = FUDGE_FACTOR * sortedPowerValues[MEDIAN_INDEX]; //gets threshold value
                //printf("%d", thresholdPowerValue);
                //loop starts at highest power, if above threshold and not ignored, then becomes hit
                uint8_t index = FILTER_FREQUENCY_COUNT - 1;
                while(sortedPowerValues[index] > thresholdPowerValue && !hitDetected){
                    if(!ignoredFreq[index]){ //set hitDetected high and select maxFreq if not ignored
                        hitDetected = true; //sets flag high
                        maxFreq = sortedIndexArray[index]; //saves index of hit
                    }
                    index--;
                }

                if(hitDetected && !ignoreAllHits){ //hitDetected and not an ignoring all frequency
                    lockoutTimer_start();
                    hitLedTimer_start();
                    detector_hitArray[maxFreq] += 1; //increases hitCount for the max Freq
                }

            }
        }
}
}

// Returns true if a hit was detected.
bool detector_hitDetected(){
    return hitDetected;
}

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit(){
    return maxFreq;
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit(){
    hitDetected = false; //lowers flag
}

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue){
    ignoreAllHits = flagValue;
}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]){
    //copies all values from detector_hitArray to passed in hitArray
    for(uint8_t filterNum = INIT_VAL; filterNum < FILTER_FREQUENCY_COUNT; filterNum++){
        hitArray[filterNum] = detector_hitArray[filterNum];
    }
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t index){
    fudgeFactorIndex = index;
}

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
detector_status_t detector_sort(uint32_t *maxPowerFreqNo, double unsortedValues[], double sortedValues[]){
    uint16_t i = 0;
    uint16_t j = 0;
    uint32_t temp = 0;
    uint32_t tempIndex = 0;
    uint32_t maxVal = 0;
    for (uint8_t m = 0; m < 10; m++) {
        sortedIndexArray[m] = m;
    }
    while (i < 10) {
        j=i;
        while((j > 0) && (unsortedValues[j-1] > unsortedValues[j])) {
            if(j == *maxPowerFreqNo) {
                *maxPowerFreqNo = j-1;
            }
            temp = unsortedValues[j-1];
            unsortedValues[j-1] = unsortedValues[j];
            unsortedValues[j] = temp;

            temp = sortedIndexArray[j-1];
            sortedIndexArray[j-1] = sortedIndexArray[j];
            sortedIndexArray[j] = temp;
            --j;
        }
        ++i;
    }
    for(uint8_t k = 0; k < 10; k++) {
        sortedValues[k] = unsortedValues[k];
    }
}

//helper function that calls detector_sort and checks ignored frequencies to return index of highest power
double detector_getHit() {

}

// Encapsulate ADC scaling for easier testing.
double detector_getScaledAdcValue(isr_AdcValue_t adcValue){
    return ((double)adcValue / ADC_SCALAR) + ADC_RANGE_ADJUST; //divide by half range, minus 1
}

/*******************************************************
 ****************** Test Routines **********************
 ******************************************************/

// Students implement this as part of Milestone 3, Task 3.
void detector_runTest(){
    //Isolated Test

    bool channels[] = {false, false, false, false, false, false, false, false, false, false};
    detector_init(channels);

    //create data set of power values that will show a hit
    unsortedPowerArray[0] = 0;
    unsortedPowerArray[1] = 2.1;
    unsortedPowerArray[2] = 100; //median
    unsortedPowerArray[3] = 78.5;
    unsortedPowerArray[4] = 9.2;
    unsortedPowerArray[5] = 0;
    unsortedPowerArray[6] = 111 * FUDGE_FACTOR; //hit on channel 6
    unsortedPowerArray[7] = 8000;
    unsortedPowerArray[8] = 900;
    unsortedPowerArray[9] = 592;

    //detector(false); //runs detector without interrupts enabled
    detector_sort(&maxFreq, unsortedPowerArray, sortedPowerValues); //sorts array
                thresholdPowerValue = FUDGE_FACTOR * sortedPowerValues[MEDIAN_INDEX]; //gets threshold value
                printf("%d", thresholdPowerValue);
                //loop starts at highest power, if above threshold and not ignored, then becomes hit
                uint8_t index = FILTER_FREQUENCY_COUNT - 1;
                while(sortedPowerValues[index] > thresholdPowerValue && !hitDetected){
                    if(!ignoredFreq[index]){ //set hitDetected high and select maxFreq if not ignored
                        hitDetected = true; //sets flag high
                        maxFreq = sortedIndexArray[index]; //saves index of hit
                    }
                    index--;
                }
    if(detector_hitDetected())
        printf("Hit on channel %d\n", maxFreq);
    else
        printf("No Hit\n");
    detector_clearHit();
    //create data set of power values that won't show power value
    unsortedPowerArray[0] = 100;
    unsortedPowerArray[1] = 2.1;
    unsortedPowerArray[2] = 100;
    unsortedPowerArray[3] = 78.5;
    unsortedPowerArray[4] = 9.2;
    unsortedPowerArray[5] = 0;
    unsortedPowerArray[6] = 11;
    unsortedPowerArray[7] = 82.9; //median
    unsortedPowerArray[8] = 90;
    unsortedPowerArray[9] = 592; //high, but not 1000 * 82.9

    //detector(false);
    detector_sort(&maxFreq, unsortedPowerArray, sortedPowerValues); //sorts array
                thresholdPowerValue = FUDGE_FACTOR * sortedPowerValues[MEDIAN_INDEX]; //gets threshold value
                printf("%d", thresholdPowerValue);
                //loop starts at highest power, if above threshold and not ignored, then becomes hit
                index = FILTER_FREQUENCY_COUNT - 1;
                while(sortedPowerValues[index] > thresholdPowerValue && !hitDetected){
                    if(!ignoredFreq[index]){ //set hitDetected high and select maxFreq if not ignored
                        hitDetected = true; //sets flag high
                        maxFreq = sortedIndexArray[index]; //saves index of hit
                    }
                    index--;
                }
    if(detector_hitDetected())
        printf("Hit on channel %d\n", maxFreq);
    else
        printf("No Hit\n");
}

// Returns 0 if passes, non-zero otherwise.
// if printTestMessages is true, print out detailed status messages.
// detector_status_t detector_testSort(sortTestFunctionPtr testSortFunction,
// bool printTestMessages);

detector_status_t detector_testAdcScaling(){

}