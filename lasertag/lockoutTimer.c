#include "lockoutTimer.h"
#include "intervalTimer.h"


//#define LOCKOUT_TIMER_EXPIRE_VALUE 50000 // Defined in terms of 100 kHz ticks.

//StateMachine States
enum lockoutTimer_st_t{
    init_st,    //start state
    wait_st,    //waits for run
    count_st    //counts for 1/2 second
};
static enum lockoutTimer_st_t lockoutTimer_currentState;
static enum lockoutTimer_st_t lockoutTimer_oldState;

//Constants
#define INIT_VAL 0


//Variables
static uint16_t lockoutTimer;
static bool run;

//Helper Functions
void printState();

// Calling this starts the timer.
void lockoutTimer_start(){
    run = true;
}

// Perform any necessary inits for the lockout timer.
void lockoutTimer_init(){
    run = false;
    lockoutTimer = INIT_VAL;
    lockoutTimer_currentState = init_st;
}

// Returns true if the timer is running.
bool lockoutTimer_running(){
    return run;
}

// Standard tick function.
void lockoutTimer_tick(){
    lockoutTimer_oldState = lockoutTimer_currentState; //saves state for transition

    //Transitions
    switch(lockoutTimer_currentState){
        case init_st:       //INIT
            lockoutTimer_currentState = wait_st;
            break;
        
        case wait_st:       //WAIT
            if(run) //run starts 1/2 sec lockout
                lockoutTimer_currentState = count_st;
            else //waits
                lockoutTimer_currentState = wait_st;
            break;
        
        case count_st:      //COUNT
            if(lockoutTimer < LOCKOUT_TIMER_EXPIRE_VALUE) //still counting
                lockoutTimer_currentState = count_st;
            else{ //1/2 sec timer done
                run = false;
                lockoutTimer = INIT_VAL;
            }
            break;
        
        default:            //DEFAULT
            lockoutTimer_currentState = init_st;
            break;
    }

    //debug function if transition occurred
    if(lockoutTimer_currentState != lockoutTimer_oldState)
        printState();

    //State Actions
    switch(lockoutTimer_currentState){
        case init_st:       //INIT
            break;
        
        case wait_st:       //WAIT
            break;
        
        case count_st:      //COUNT
            lockoutTimer++;
            break;
        
        default:            //DEFAULT
            break;
    }
}

//Helper Functions

void printState(){ //prints current state
    switch(lockoutTimer_currentState){ //print for each state
        case init_st:       //INIT
            printf("init\n");
            break;
        
        case wait_st:       //WAIT
            printf("wait\n");
            break;
        
        case count_st:      //COUNT
            printf("count\n");
            break;
        
        default:            //DEFAULT
            printf("Error - Default\n");
            break;
    }
}

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest(){
    intervalTimer_init(INTERVAL_TIMER_TIMER_1); //using Timer 1
    intervalTimer_start(INTERVAL_TIMER_TIMER_1); //start
    
    //Run Lockout Timer
    lockoutTimer_start();
    while(lockoutTimer_running());

    intervalTimer_stop(INTERVAL_TIMER_TIMER_1); //stop timer
    printf("Lockout Timer: %f", intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_TIMER_1));
}