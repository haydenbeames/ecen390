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

//Constants
#define INIT_VAL 0
#define IGNORED_FREQUENCY_SIZE
static uint16_t ignoredFreq[IGNORED_FREQUENCY_SIZE];
// Always have to init things.
// bool array is indexed by frequency number, array location set for true to
// ignore, false otherwise. This way you can ignore multiple frequencies.
void detector_init(bool ignoredFrequencies[]){
    filter_init();
    adcBufferInit();
    for(uint8_t i = 0; i < IGNORED_FREQUENCY_SIZE; i++) {
        ignoredFreq[i] = ignoredFrequencies[i];
    }
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
    uint32_t elementCount = isr_adcBufferElemntCount();
    uint32_t rawAdcValue = INIT_VAL;
    bool interruptsEnabled = false;
    double scaledAdcValue = INIT_VAL;
    for(uint32_t i = INIT_VAL; i < elemntCount; i++){ //repeats for all elements
        if(interruptsEnabled) //disables interrupts to safely manipulate adcBuffer
            interrupts_disableArmInts();
        
        rawAdcBuffer = isr_removeDataFromAdcBuffer(); //pop value
        if(interruptsEnabled) //reinstates interrupts if going before
            interrupts_enableArmInts();
        scaledAdcValue;
    }
}

// Returns true if a hit was detected.
bool detector_hitDetected(){
    return interruptsCurrentlyEnabled;
}

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit(){
    return 
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit(){

}

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue){

}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]){

}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t){

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

}

// Encapsulate ADC scaling for easier testing.
double detector_getScaledAdcValue(isr_AdcValue_t adcValue){

}

/*******************************************************
 ****************** Test Routines **********************
 ******************************************************/

// Students implement this as part of Milestone 3, Task 3.
void detector_runTest(){

}

// Returns 0 if passes, non-zero otherwise.
// if printTestMessages is true, print out detailed status messages.
// detector_status_t detector_testSort(sortTestFunctionPtr testSortFunction,
// bool printTestMessages);

detector_status_t detector_testAdcScaling(){

}

