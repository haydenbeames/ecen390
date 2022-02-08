#include "filter.h"
#include <stdio.h>
/*
#define FILTER_SAMPLE_FREQUENCY_IN_KHZ 100
#define FILTER_FREQUENCY_COUNT 10
#define FILTER_FIR_DECIMATION_FACTOR                                           \
  10 // FIR-filter needs this many new inputs to compute a new output.
#define FILTER_INPUT_PULSE_WIDTH                                               \
  2000 // This is the width of the pulse you are looking for, in terms of
       // decimated sample count.
// These are the tick counts that are used to generate the user frequencies.
// Not used in filter.h but are used to TEST the filter code.
// Placed here for general access as they are essentially constant throughout
// the code. The transmitter will also use these.
static const uint16_t filter_frequencyTickTable[FILTER_FREQUENCY_COUNT] = {
    68, 58, 50, 44, 38, 34, 30, 28, 26, 24};

// Filtering routines for the laser-tag project.
// Filtering is performed by a two-stage filter, as described below.

// 1. First filter is a decimating FIR filter with a configurable number of taps
// and decimation factor.
// 2. The output from the decimating FIR filter is passed through a bank of 10
// IIR filters. The characteristics of the IIR filter are fixed.*/

/*********************************************************************************************************
****************************************** Main Filter Functions
******************************************
**********************************************************************************************************/
// Constants
#define INIT_VAL 0
#define INIT_VAL_DOUBLE 0.0

#define NUM_IIR_FILTERS 10      //Queue constants
#define QUEUE_INIT_VAL 0.0
#define X_QUEUE_SIZE 20000                      //not sure yet
#define Y_QUEUE_SIZE 20000                      //not sure yet
#define Z_QUEUE_SIZE IIR_A_COEF_COUNT    //confirmed
#define OUTPUT_QUEUE_SIZE 2000                  //confirmed

#define IIR_A_COEF_COUNT 10 //ignores first coefficient
#define IIR_B_COEF_COUNT 11
#define FIR_COEF_COUNT 81

#define FIR_DECIMATION_FACTOR 10



//Queues
static queue_t xQueue;
static queue_t yQueue;
static queue_t zQueue[NUM_IIR_FILTERS];
static queue_t outputQueue[NUM_IIR_FILTERS];

//Filter Coefficients
const static double fir_coef[FIR_COEF_COUNT] = {0.0, 3.8342823682706231e-06, 1.8434064577496714e-05, 4.3060491068262203e-05,   6.8631079589207919e-05,   7.8200362324506827e-05,   5.0102879271319798e-05,  -3.6266988484992889e-05,  -1.9270375257708750e-04,  -4.1387385496878429e-04,  -6.7051426702074805e-04,  -9.0661140624107158e-04,  -1.0428833238331600e-03,  -9.8805880576526180e-04,  -6.5791632788934223e-04,   8.0500245874684433e-19,   9.8024085406310632e-04,   2.1978984227582011e-03,   3.4784175278154703e-03,   4.5648660041106197e-03,   5.1464217547564665e-03,   4.9080082807048840e-03,   3.5962648215440043e-03,   1.0922779580756008e-03,  -2.5221861909812873e-03,  -6.9197914522424403e-03,  -1.1517667555764310e-02,  -1.5511646188481250e-02,  -1.7957031761487947e-02,  -1.7889731262750234e-02,  -1.4473018146767496e-02,  -7.1483446400393445e-03,   4.2349826871505942e-03,   1.9335722047565503e-02,   3.7301326020617906e-02,   5.6823053849101399e-02,   7.6263450588519716e-02,   9.3841702665884402e-02,   1.0785179614821015e-01,   1.1688166247967095e-01,   1.1999999999999998e-01,   1.1688166247967095e-01,   1.0785179614821015e-01,   9.3841702665884402e-02,   7.6263450588519716e-02,   5.6823053849101399e-02,   3.7301326020617906e-02,   1.9335722047565503e-02,   4.2349826871505942e-03,  -7.1483446400393445e-03,  -1.4473018146767496e-02,  -1.7889731262750234e-02,  -1.7957031761487947e-02,  -1.5511646188481250e-02,  -1.1517667555764310e-02,  -6.9197914522424403e-03,  -2.5221861909812873e-03,   1.0922779580756008e-03,   3.5962648215440043e-03,   4.9080082807048840e-03,   5.1464217547564665e-03,   4.5648660041106197e-03,   3.4784175278154703e-03,   2.1978984227582011e-03,   9.8024085406310632e-04,   8.0500245874684433e-19,  -6.5791632788934223e-04,  -9.8805880576526180e-04,  -1.0428833238331600e-03,  -9.0661140624107158e-04,  -6.7051426702074805e-04,  -4.1387385496878429e-04,  -1.9270375257708750e-04,  -3.6266988484992889e-05,   5.0102879271319798e-05,   7.8200362324506827e-05,   6.8631079589207919e-05,   4.3060491068262203e-05,   1.8434064577496714e-05,   3.8342823682706231e-06,   0.0000000000000000e+00};

const static double iir_a_coef[NUM_IIR_FILTERS][IIR_A_COEF_COUNT] = {{-5.9637727070164015e+00,   1.9125339333078248e+01,  -4.0341474540744173e+01,   6.1537466875368821e+01,  -7.0019717951472188e+01,   6.0298814235238872e+01,  -3.8733792862566290e+01,   1.7993533279581058e+01,  -5.4979061224867651e+00,   9.0332828533799547e-01},   //channel 1
                                                                    {-4.6377947119071505e+00,   1.3502215749461598e+01,  -2.6155952405269829e+01,   3.8589668330738476e+01,  -4.3038990303252795e+01,   3.7812927599537275e+01,  -2.5113598088113893e+01,   1.2703182701888142e+01,  -4.2755083391143689e+00,   9.0332828533800569e-01},    //channel 2
                                                                    {-3.0591317915750973e+00,   8.6417489609637634e+00,  -1.4278790253808870e+01,   2.1302268283304350e+01,  -2.2193853972079282e+01,   2.0873499791105495e+01,  -1.3709764520609433e+01,   8.1303553577931904e+00,  -2.8201643879900606e+00,   9.0332828533800258e-01},    //channel 3
                                                                    {-1.4071749185996769e+00,   5.6904141470697596e+00,  -5.7374718273676431e+00,   1.1958028362868930e+01,  -8.5435280598354897e+00,   1.1717345583836000e+01,  -5.5088290876998869e+00,   5.3536787286077878e+00,  -1.2972519209655655e+00,   9.0332828533800436e-01},    //channel 4
                                                                    {8.2010906117760696e-01,   5.1673756579268639e+00,   3.2580350909221076e+00,   1.0392903763919204e+01,   4.8101776408669306e+00,   1.0183724507092521e+01,   3.1282000712126905e+00,   4.8615933365572053e+00,   7.5604535083145297e-01,   9.0332828533800136e-01},     //channel 5
                                                                    {2.7080869856154512e+00,   7.8319071217995688e+00,   1.2201607990980744e+01,   1.8651500443681620e+01,   1.8758157568004549e+01,   1.8276088095999022e+01,   1.1715361303018897e+01,   7.3684394621253499e+00,   2.4965418284511904e+00,   9.0332828533800436e-01},     //channel 6
                                                                    {4.9479835250075892e+00,   1.4691607003177602e+01,   2.9082414772101060e+01,   4.3179839108869331e+01,   4.8440791644688879e+01,   4.2310703962394342e+01,   2.7923434247706432e+01,   1.3822186510471010e+01,   4.5614664160654357e+00,   9.0332828533799958e-01},     //channel 7
                                                                    {6.1701893352279846e+00,   2.0127225876810336e+01,   4.2974193398071684e+01,   6.5958045321253451e+01,   7.5230437667866596e+01,   6.4630411355739852e+01,   4.1261591079244127e+01,   1.8936128791950534e+01,   5.6881982915180291e+00,   9.0332828533799803e-01},     //channel 8
                                                                    {7.4092912870072398e+00,   2.6857944460290135e+01,   6.1578787811202247e+01,   9.8258255839887312e+01,   1.1359460153696298e+02,   9.6280452143026082e+01,   5.9124742025776392e+01,   2.5268527576524203e+01,   6.8305064480743081e+00,   9.0332828533799969e-01},     //channel 9
                                                                    {8.5743055776347692e+00,   3.4306584753117889e+01,   8.4035290411037053e+01,   1.3928510844056814e+02,   1.6305115418161620e+02,   1.3648147221895786e+02,   8.0686288623299745e+01,   3.2276361903872115e+01,   7.9045143816244696e+00,   9.0332828533799636e-01}};    //channel 10

const static double iir_b_coef[NUM_IIR_FILTERS][IIR_B_COEF_COUNT] = {{ 9.0928661148194738e-10,   0.0000000000000000e+00,  -4.5464330574097372e-09,   0.0000000000000000e+00,   9.0928661148194745e-09,   0.0000000000000000e+00,  -9.0928661148194745e-09,   0.0000000000000000e+00,   4.5464330574097372e-09,   0.0000000000000000e+00,  -9.0928661148194738e-10},   //channel 1
                                                                    {9.0928661148175165e-10,   0.0000000000000000e+00,  -4.5464330574087587e-09,   0.0000000000000000e+00,   9.0928661148175174e-09,   0.0000000000000000e+00,  -9.0928661148175174e-09,   0.0000000000000000e+00,   4.5464330574087587e-09,   0.0000000000000000e+00,  -9.0928661148175165e-10},    //channel 2
                                                                    { 9.0928661148192040e-10,   0.0000000000000000e+00,  -4.5464330574096024e-09,   0.0000000000000000e+00,   9.0928661148192048e-09,   0.0000000000000000e+00,  -9.0928661148192048e-09,   0.0000000000000000e+00,   4.5464330574096024e-09,   0.0000000000000000e+00,  -9.0928661148192040e-10},    //channel 3
                                                                    { 9.0928661148191275e-10,   0.0000000000000000e+00,  -4.5464330574095635e-09,   0.0000000000000000e+00,   9.0928661148191270e-09,   0.0000000000000000e+00,  -9.0928661148191270e-09,   0.0000000000000000e+00,   4.5464330574095635e-09,   0.0000000000000000e+00,  -9.0928661148191275e-10},    //channel 4
                                                                    {9.0928661148211809e-10,   0.0000000000000000e+00,  -4.5464330574105901e-09,   0.0000000000000000e+00,   9.0928661148211801e-09,   0.0000000000000000e+00,  -9.0928661148211801e-09,   0.0000000000000000e+00,   4.5464330574105901e-09,   0.0000000000000000e+00,  -9.0928661148211809e-10},     //channel 5
                                                                    {9.0928661148179839e-10,   0.0000000000000000e+00,  -4.5464330574089919e-09,   0.0000000000000000e+00,   9.0928661148179839e-09,   0.0000000000000000e+00,  -9.0928661148179839e-09,   0.0000000000000000e+00,   4.5464330574089919e-09,   0.0000000000000000e+00,  -9.0928661148179839e-10},     //channel 6
                                                                    {9.0928661148193684e-10,   0.0000000000000000e+00,  -4.5464330574096843e-09,   0.0000000000000000e+00,   9.0928661148193686e-09,   0.0000000000000000e+00,  -9.0928661148193686e-09,   0.0000000000000000e+00,   4.5464330574096843e-09,   0.0000000000000000e+00,  -9.0928661148193684e-10},     //channel 7
                                                                    {9.0928661148195069e-10,   0.0000000000000000e+00,  -4.5464330574097538e-09,   0.0000000000000000e+00,   9.0928661148195076e-09,   0.0000000000000000e+00,  -9.0928661148195076e-09,   0.0000000000000000e+00,   4.5464330574097538e-09,   0.0000000000000000e+00,  -9.0928661148195069e-10},     //channel 8
                                                                    {9.0928661148190954e-10,   0.0000000000000000e+00,  -4.5464330574095478e-09,   0.0000000000000000e+00,   9.0928661148190956e-09,   0.0000000000000000e+00,  -9.0928661148190956e-09,   0.0000000000000000e+00,   4.5464330574095478e-09,   0.0000000000000000e+00,  -9.0928661148190954e-10},     //channel 9
                                                                    {9.0928661148206091e-10,   0.0000000000000000e+00,  -4.5464330574103047e-09,   0.0000000000000000e+00,   9.0928661148206094e-09,   0.0000000000000000e+00,  -9.0928661148206094e-09,   0.0000000000000000e+00,   4.5464330574103047e-09,   0.0000000000000000e+00,  -9.0928661148206091e-10}};    //channel 10

//array for current power values - initialized to zero
static double currentPowerValue[NUM_IIR_FILTERS] = {INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE};
//saves oldest value to subtract from power - efficient power calculation
static double oldestValue[NUM_IIR_FILTERS] = {INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE, INIT_VAL_DOUBLE};


//generic init for all queues and fills with zeros, takes in size of queue
void initQueue(queue_t* q, uint32_t queueSize, char* name){
    queue_init(q, queueSize, name);
    for(uint32_t i = INIT_VAL; i < queueSize; i++){ //fills with default value
        queue_overwritePush(q, QUEUE_INIT_VAL);
    }
}

//inits the xQueue using initQueue
void initXQueue(){
    initQueue(&xQueue, X_QUEUE_SIZE, "x");
}

//inits the yQueue using initQueue
void initYQueue(){
    initQueue(&yQueue, Y_QUEUE_SIZE, "y");
}

// inits all 10 IIR Filter zQueues
void initZQueue(){
    char* name;
    for(uint32_t i = INIT_VAL; i < NUM_IIR_FILTERS; i++) { //makes each queue instance
        sprintf(name, "z%d", i);
        //zQueues[i] = queue_t q;
        initQueue(&(zQueue[i]), Z_QUEUE_SIZE, name);
    }
}

// inits all 10 Output Filter zQueues
void initOutputQueue() {
    char* name;
    for(uint32_t i = INIT_VAL; i < NUM_IIR_FILTERS; i++) { //makes each queue instance
        sprintf(name, "output%d", i);
        //outputQueue[i] = queue_t q;
        initQueue(&(zQueue[i]), OUTPUT_QUEUE_SIZE, name);
    }
}


// Must call this prior to using any filter functions.
void filter_init(){
    initXQueue(); //inits each queue and fills with 0.0
    initYQueue();
    initZQueue();
    initOutputQueue();
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x){
    queue_push(&xQueue, x);
}

// Fills a queue with the given fillValue. For example,
// if the queue is of size 10, and the fillValue = 1.0,
// after executing this function, the queue will contain 10 values
// all of them 1.0.
void filter_fillQueue(queue_t *q, double fillValue){
    queue_size_t queueSize = queue_size(q);
    for(queue_size_t i = INIT_VAL; i < queueSize; i++){ //fills with default value
        queue_overwritePush(&q, fillValue);
    }
}

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter() {
    double y = 0.0;
    for(uint32_t i = 0; i < Y_QUEUE_SIZE; i++) {
       y += queue_readElementAt(&xQueue, FIR_COEF_COUNT-1-i) * fir_coef[i];
    }
    queue_overwritePush(&yQueue, y);
    return y;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber){
    double b_coef_sum = INIT_VAL_DOUBLE;
    double a_coef_sum = INIT_VAL_DOUBLE;
    double total_sum;
    
    //IIR B coefficients - iteratively adds coef * y val
    for(uint32_t i = INIT_VAL; i < IIR_B_COEF_COUNT; i++){
        b_coef_sum += iir_b_coef[filterNumber][i] * queue_readElementAt(&yQueue, IIR_B_COEF_COUNT -1 -i);
    }

    //IIR A coefficients - iteratively adds coef * z val
    for(uint32_t i = INIT_VAL; i < IIR_A_COEF_COUNT; i++){
        a_coef_sum += iir_a_coef[filterNumber][i] * queue_readElementAt(&(zQueue[filterNumber]), IIR_A_COEF_COUNT -1 -i);
    }
    total_sum = b_coef_sum - a_coef_sum;

    queue_push(&(zQueue[filterNumber]), total_sum);
    queue_push(&(outputQueue[filterNumber]), total_sum);
    return total_sum;
}

// Use this to compute the power for values contained in an outputQueue.
// If force == true, then recompute power by using all values in the
// outputQueue. This option is necessary so that you can correctly compute power
// values the first time. After that, you can incrementally compute power values
// by:
// 1. Keeping track of the power computed in a previous run, call this
// prev-power.
// 2. Keeping track of the oldest outputQueue value used in a previous run, call
// this oldest-value.
// 3. Get the newest value from the power queue, call this newest-value.
// 4. Compute new power as: prev-power - (oldest-value * oldest-value) +
// (newest-value * newest-value). Note that this function will probably need an
// array to keep track of these values for each of the 10 output queues.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch, bool debugPrint){
    double power_sum = INIT_VAL_DOUBLE;
    double currVal;
    double newVal;
    double oldVal;
    uint32_t qLength = queue_elementCount(&(outputQueue[filterNumber])); //access length once
    //force case
    if(forceComputeFromScratch){
        //iterates through length of queue to add the square of each element
        for(uint32_t i = INIT_VAL; i < qLength; i++){
            currVal = queue_readElementAt(&(outputQueue[filterNumber]),i); //access queue once
            power_sum += (currVal * currVal); //summing currVal^2
        }
        
    }
    else{ //regular calculation
        newVal = queue_readElementAt(&(outputQueue[filterNumber]),INIT_VAL); //pulls newest value
        oldVal = oldestValue[filterNumber]; //pulls oldest value (not pushed off queue)
        power_sum = currentPowerValue[filterNumber];
        power_sum += ((newVal * newVal) - (oldVal * oldVal)); //adds newest square and subtracts oldest sum
    }

    oldestValue[filterNumber] = queue_readElementAt(&(outputQueue[filterNumber]), qLength - 1); //saves oldest index for next power calculation
    currentPowerValue[filterNumber] = power_sum; //save power sum to array
    return power_sum;
}

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber){
    return currentPowerValue[filterNumber];
}

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes to
// the array within that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[]){
    for(uint32_t i = INIT_VAL; i < NUM_IIR_FILTERS; i++){
        powerValues[i] = currentPowerValue[i];
    }
}

// Using the previously-computed power values that are current stored in
// currentPowerValue[] array, Copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in currentPowerValue[].
void filter_getNormalizedPowerValues(double normalizedArray[], uint16_t *indexOfMaxValue){
    for(uint32_t i = 0; i < NUM_IIR_FILTERS; i++) {
        normalizedArray[i] = currentPowerValue[i];
    }
    for(uint32_t i = 0; i < NUM_IIR_FILTERS; i++) {
        normalizedArray[i] = normalizedArray[i] / currentPowerValue[*indexOfMaxValue];
    }
}

/*********************************************************************************************************
********************************** Verification-assisting functions.
**************************************
********* Test functions access the internal data structures of the filter.c via
*these functions. ********
*********************** These functions are not used by the main filter
*functions. ***********************
**********************************************************************************************************/

const double *filter_getFirCoefficientArray(){
    return fir_coef;
}

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount(){
    return FIR_COEF_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber){
    return iir_a_coef[filterNumber];
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount(){
    return IIR_A_COEF_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber){
    return iir_b_coef[filterNumber];
}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount(){
    return IIR_B_COEF_COUNT;
}

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize() {
    return queue_elementCount(&yQueue);
}

// Returns the decimation value.
uint16_t filter_getDecimationValue(){
    return FIR_DECIMATION_FACTOR;

}

// Returns the address of xQueue.
queue_t *filter_getXQueue(){
    return &xQueue;
}

// Returns the address of yQueue.
queue_t *filter_getYQueue(){
    return &yQueue;
}

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber){
    return &(zQueue[filterNumber]);
}

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber){
    return &(outputQueue[filterNumber]);
}

// void filter_runTest();
