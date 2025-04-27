/*
 * state machine
 * handles the various transitions and functions of the box
 * this state machine controls how the phone lock box behaves,
 * transitioning between different states based on flags and events.
 */

#include "state_machine.h"

#include <stdio.h>
#include <stdlib.h>
#include "Screen_Driver.h"
#include "accelerometer.h"
#include "audio.h"
#include "event_controller.h"
#include "nfc.h"
#include "rotary_encoder.h"
#include "shared.h"
#include "main.h"
#include "lock_timer.h"

#define MINUTE 60000  // constant for 1 minute in milliseconds

BoxState previous;  // stores the previous state
BoxState state;     // current state of the box
SFlag flags[MAX_FLAGS];  // array for storing flags used in state transitions
bool master_timer_done;  // flag to track when the master timer is done

// checks if a flag exists in the flags array
bool hasFlag(SFlag flag) {
    for (uint8_t i = 0; i < MAX_FLAGS; ++i) {
        if (flags[i] == flag) return true;  // return true if flag is found
    }
    return false;  // return false if flag isn't found
}

// adds a flag to the flags array if it's not already present
bool stateInsertFlag(SFlag flag) {
#ifdef DEBUG_STATE_CONTROLLER
    printf("[INFO] inserting state flag: %s\n\r", SFlagToStr(flag));
#endif

    uint8_t empty_idx = MAX_FLAGS;
    // search for an empty spot to insert the flag
    for(uint8_t i = 0; i < MAX_FLAGS; ++i) {
        if (flags[i] == flag) {
            return false;  // flag is already in the array
        } else if (flags[i] == SFLAG_NULL && i < empty_idx) {
            empty_idx = i;  // find the first available empty spot
        }
    }

    if (empty_idx < MAX_FLAGS) {
        flags[empty_idx] = flag;  // insert the flag into the empty spot
        return true;  // flag inserted successfully
    }

    return false;  // no available spots to insert the flag
}

// clears all flags by resetting them to SFLAG_NULL
void clearFlags(void) {
    for (uint8_t i = 0; i < MAX_FLAGS; ++i) {
        flags[i] = SFLAG_NULL;  // reset each flag to null
    }
}

// enables or disables interrupts based on the current state
void inturruptControl(void) {
    switch(state) {
        // states where the button interrupt needs to be disabled
        case UNLOCKED_ASLEEP_TO_AWAKE:
        case UNLOCKED_FULL_AWAKE_FUNC_A:
        case LOCKED_FULL_AWAKE:
        case LOCKED_MONITOR_AWAKE:
        case LOCKED_MONITOR_ASLEEP:
        case EMERGENCY_OPEN:
            HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);  // disables button interrupt
            break;

        // states where the button interrupt needs to be enabled
        case UNLOCKED_EMPTY_ASLEEP:
        case UNLOCKED_EMPTY_AWAKE:
        case UNLOCKED_FULL_ASLEEP:
        case UNLOCKED_FULL_AWAKE_FUNC_B:
        case UNLOCKED_TO_LOCKED_AWAKE:
        case LOCKED_FULL_NOTIFICATION_FUNC_A:
        case LOCKED_FULL_NOTIFICATION_FUNC_B:
        case LOCKED_FULL_ASLEEP:
            HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);  // enables button interrupt
            break;
    }

    switch(state) {
        // states where the audio interrupt needs to be disabled
        case UNLOCKED_EMPTY_ASLEEP:
        case UNLOCKED_ASLEEP_TO_AWAKE:
        case UNLOCKED_EMPTY_AWAKE:
        case UNLOCKED_FULL_AWAKE_FUNC_A:
        case UNLOCKED_FULL_AWAKE_FUNC_B:
        case UNLOCKED_FULL_ASLEEP:
        case UNLOCKED_TO_LOCKED_AWAKE:
        case LOCKED_FULL_NOTIFICATION_FUNC_A:
        case LOCKED_FULL_NOTIFICATION_FUNC_B:
        case EMERGENCY_OPEN:
            HAL_NVIC_DisableIRQ(EXTI0_IRQn);  // disables audio interrupt
            break;

        // states where the audio interrupt needs to be enabled
        case LOCKED_MONITOR_AWAKE:
        case LOCKED_FULL_ASLEEP:
        case LOCKED_FULL_AWAKE:
        case LOCKED_MONITOR_ASLEEP:
            HAL_NVIC_EnableIRQ(EXTI0_IRQn);  // enables audio interrupt
            break;
    }
}

// initializes the state machine and sets the initial state
void stateMachineInit(void) {
    state = UNLOCKED_EMPTY_ASLEEP;  // set the initial state to UNLOCKED_EMPTY_ASLEEP
}

// main function for running the state machine and handling transitions
void runStateMachine(void) {
    BoxState next = state;  // variable to store the next state

    switch (state) {
        // state transitions for UNLOCKED_EMPTY_ASLEEP
        case UNLOCKED_EMPTY_ASLEEP:
            if (hasFlag(SFLAG_ACC_BOX_MOVED) || hasFlag(SFLAG_ROTENC_INTERRUPT) || hasFlag(SFLAG_ROTENC_ROTATED)) {
                next = UNLOCKED_ASLEEP_TO_AWAKE;  // move to awake state if the box is moved or rotary encoder is triggered
            }
            break;

        // state transitions for UNLOCKED_ASLEEP_TO_AWAKE
        case UNLOCKED_ASLEEP_TO_AWAKE:
            if (hasFlag(SFLAG_TIMER_COMPLETE)) {
                next = UNLOCKED_EMPTY_AWAKE;  // after timer completes, move to awake state
            }
            break;

        // state transitions for UNLOCKED_EMPTY_AWAKE
        case UNLOCKED_EMPTY_AWAKE:
            if (hasFlag(SFLAG_NFC_PHONE_PRESENT)) {
                next = UNLOCKED_FULL_AWAKE_FUNC_A;  // move to full awake if phone is detected
            }
            else if (hasFlag(SFLAG_TIMER_COMPLETE) || hasFlag(SFLAG_ROTENC_INTERRUPT)) {
                next = UNLOCKED_EMPTY_ASLEEP;  // move to sleep if timer completes or rotary encoder is triggered
            }
            break;

        // state transitions for UNLOCKED_FULL_AWAKE_FUNC_A
        case UNLOCKED_FULL_AWAKE_FUNC_A:
            if (hasFlag(SFLAG_NFC_PHONE_NOT_PRESENT)) {
                next = UNLOCKED_EMPTY_AWAKE;  // move to awake state if phone is removed
            }
            else if (hasFlag(SFLAG_ROTENC_ROTATED)) {
                next = UNLOCKED_FULL_AWAKE_FUNC_B;  // move to func B if rotary encoder is rotated
            } else if (hasFlag(SFLAG_TIMER_COMPLETE) || hasFlag(SFLAG_ROTENC_INTERRUPT)) {
                next = UNLOCKED_FULL_ASLEEP;  // move to sleep state if timer completes or rotary encoder is triggered
            }
            break;

        // state transitions for UNLOCKED_FULL_AWAKE_FUNC_B
        case UNLOCKED_FULL_AWAKE_FUNC_B:
            if (hasFlag(SFLAG_ROTENC_INTERRUPT) && hasFlag(SFLAG_BOX_CLOSED)) {
                next = UNLOCKED_TO_LOCKED_AWAKE;  // transition to locked if box is closed and rotary encoder is triggered
            }
            else if (hasFlag(SFLAG_ROTENC_ROTATED)) {
                next = UNLOCKED_FULL_AWAKE_FUNC_A;  // go back to func A if rotary encoder is rotated
            } else if (hasFlag(SFLAG_NFC_PHONE_NOT_PRESENT)) {
                next = UNLOCKED_EMPTY_AWAKE;  // move back to awake state if phone is not present
            } else if (hasFlag(SFLAG_TIMER_COMPLETE)) {
                next = UNLOCKED_FULL_ASLEEP;  // move to sleep state if timer completes
            }
            break;

        // state transitions for UNLOCKED_FULL_ASLEEP
        case UNLOCKED_FULL_ASLEEP:
            if (hasFlag(SFLAG_NFC_PHONE_NOT_PRESENT)) {
                next = UNLOCKED_EMPTY_AWAKE;  // move to awake state if phone is not present
            }
            else if (hasFlag(SFLAG_ACC_BOX_MOVED) || hasFlag(SFLAG_ROTENC_ROTATED) || hasFlag(SFLAG_ROTENC_INTERRUPT)) {
                next = UNLOCKED_FULL_AWAKE_FUNC_A;  // move to full awake if box is moved or rotary encoder is triggered
            }
            break;

        // state transitions for UNLOCKED_TO_LOCKED_AWAKE
        case UNLOCKED_TO_LOCKED_AWAKE:
            if ((hasFlag(SFLAG_NFC_PHONE_NOT_PRESENT)) || hasFlag(SFLAG_BOX_OPEN) || hasFlag(SFLAG_ROTENC_INTERRUPT)) {
                next = UNLOCKED_FULL_AWAKE_FUNC_B;  // move to awake state if box is open or phone is removed
            } else if (hasFlag(SFLAG_TIMER_COMPLETE)) {
                next = LOCKED_FULL_AWAKE;  // move to locked state after timer completes
            }
            break;

        // state transitions for LOCKED_FULL_AWAKE
        case LOCKED_FULL_AWAKE:
            if (hasFlag(SFLAG_BOX_OPEN)) {
                next = EMERGENCY_OPEN;  // enter emergency open state if the box is open
            }
            else if (hasFlag(SFLAG_TIMER_COMPLETE)) {
                next = LOCKED_FULL_ASLEEP;  // move to asleep state if timer completes
            }
            else if (hasFlag(SFLAG_AUDIO_VOL_HIGH)) {
                next = LOCKED_MONITOR_AWAKE;  // move to monitor awake if audio is detected
            }
            break;

        // state transitions for LOCKED_FULL_NOTIFICATION_FUNC_A
        case LOCKED_FULL_NOTIFICATION_FUNC_A:
            if (hasFlag(SFLAG_BOX_OPEN)) {
                next = EMERGENCY_OPEN;  // emergency open if the box is open
            }
            else if (hasFlag(SFLAG_ROTENC_INTERRUPT)) {
                next = UNLOCKED_FULL_AWAKE_FUNC_B;  // unlock if rotary encoder interrupt is detected
            }
            else if (hasFlag(SFLAG_ROTENC_ROTATED)) {
                next = LOCKED_FULL_NOTIFICATION_FUNC_B;  // move to next notification function if rotary encoder is rotated
            }
            else if (hasFlag(SFLAG_TIMER_COMPLETE)) {
                next = LOCKED_FULL_AWAKE;  // move back to awake state if timer completes
            }
            break;

        // state transitions for LOCKED_FULL_NOTIFICATION_FUNC_B
        case LOCKED_FULL_NOTIFICATION_FUNC_B:
            if (hasFlag(SFLAG_BOX_OPEN)) {
                next = EMERGENCY_OPEN;  // emergency open if box is open
            }
            else if (hasFlag(SFLAG_ROTENC_ROTATED)) {
                next = LOCKED_FULL_NOTIFICATION_FUNC_A;  // go back to func A if rotary encoder is rotated
            }
            else if (hasFlag(SFLAG_TIMER_COMPLETE)) {
                next = LOCKED_FULL_AWAKE;  // move to awake state if timer completes
            }
            break;

        // state transitions for LOCKED_FULL_ASLEEP
        case LOCKED_FULL_ASLEEP:
            if (hasFlag(SFLAG_BOX_OPEN)) {
                next = EMERGENCY_OPEN;  // emergency open if box is open
            }
            else if (hasFlag(SFLAG_AUDIO_VOL_HIGH)) {
                next = LOCKED_MONITOR_ASLEEP;  // move to monitor asleep if high audio is detected
            }
            else if (hasFlag(SFLAG_ROTENC_INTERRUPT) || hasFlag(SFLAG_ROTENC_ROTATED) || hasFlag(SFLAG_ACC_BOX_MOVED)) {
                next = LOCKED_FULL_AWAKE;  // move to awake state if rotary encoder is triggered or box is moved
            }
            break;

        // state transitions for LOCKED_MONITOR_AWAKE
        case LOCKED_MONITOR_AWAKE:
            if (hasFlag(SFLAG_BOX_OPEN)) {
                next = EMERGENCY_OPEN;  // emergency open if box is open
            }
            else if (hasFlag(SFLAG_AUDIO_MATCH)) {
                next = LOCKED_FULL_NOTIFICATION_FUNC_B;  // move to notification if audio match is found
            } else if (hasFlag(SFLAG_AUDIO_NO_MATCH)) {
                next = LOCKED_FULL_AWAKE;  // go back to full awake if no audio match is found
            }
            break;

        // state transitions for LOCKED_MONITOR_ASLEEP
        case LOCKED_MONITOR_ASLEEP:
            if (hasFlag(SFLAG_BOX_OPEN)) {
                next = EMERGENCY_OPEN;  // emergency open if box is open
            }
            else if (hasFlag(SFLAG_AUDIO_MATCH)) {
                next = LOCKED_FULL_NOTIFICATION_FUNC_B;  // go to notification if audio match is found
            }
            else if (hasFlag(SFLAG_AUDIO_NO_MATCH)) {
                next = LOCKED_FULL_ASLEEP;  // go back to sleep if no audio match is found
            }
            break;

        // state transitions for EMERGENCY_OPEN
        case EMERGENCY_OPEN:
            if (hasFlag(SFLAG_TIMER_COMPLETE)) {
                next = UNLOCKED_FULL_AWAKE_FUNC_B;  // transition back to awake func B after emergency open
            }
            break;

        default:
#ifdef DEBUG_STATE_CONTROLLER
            printf("[ERROR] default case of state machine reached, system in bad state\n\r");
#endif
            break;
    }

    // if the master timer is done, reset to full awake state
    if (master_timer_done) {
        next = UNLOCKED_FULL_AWAKE_FUNC_A;
        master_timer_done = false;
    }

    // update state if it changes
    if (next != state) {
        eventClear();  // clear all events associated with the current state
        clearFlags();  // reset all flags

#ifdef DEBUG_STATE_CONTROLLER
        printf("\n[Info] --- state transition: %s → %s ---\n\r", stateToStr(state), stateToStr(next));
#endif

        stateTransitionCleanup(next);  // handle cleanup when transitioning to the next state

        previous = state;
        state = next;

        // setup the new state
        inturruptControl();  // handle interrupt enabling/disabling
        stateScheduleEvents();  // schedule events for the new state
        screenResolve();  // update the screen if needed
    }
}

// transition specific logic based on the current state and the next state
void stateTransitionCleanup(BoxState next) {
    // if transitioning from unlocked awake to locked awake, start the lock timer and engage the lock
    if (state == UNLOCKED_TO_LOCKED_AWAKE && next == LOCKED_FULL_AWAKE) {
        lockTimerStart();  // start the lock timer
        lockEngage();      // engage the lock
    }
    // if transitioning from locked awake or locked asleep to unlocked awake function B, cancel the lock timer and disengage the lock
    else if ((state == LOCKED_FULL_AWAKE || state == LOCKED_FULL_ASLEEP) && next == UNLOCKED_FULL_AWAKE_FUNC_B) {
        lockTimerCancel();  // cancel the lock timer
        lockDisenage();     // disengage the lock
    }
    // if transitioning to emergency open, cancel the lock timer and disengage the lock
    else if (next == EMERGENCY_OPEN) {
        lockTimerCancel();  // cancel the lock timer
        lockDisenage();     // disengage the lock
    }
}

// schedules events based on the current state of the box
void stateScheduleEvents() {
    switch (state) {
        case UNLOCKED_EMPTY_ASLEEP:
            // schedule accelerometer and rotary encoder events to detect box movement and user interaction
            eventRegister(accDeltaEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 10, 0);
            eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
            break;

        case UNLOCKED_ASLEEP_TO_AWAKE:
            // schedule a timer event to transition after 1 second
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, 1000, 0);
            break;

        case UNLOCKED_EMPTY_AWAKE:
            // schedule NFC event to detect phone and timer event to transition after 1 minute
            eventRegister(nfcEventCallbackSlow, EVENT_NFC_READ, EVENT_DELTA, 1000, 0);
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
            break;

        case UNLOCKED_FULL_AWAKE_FUNC_A:
            // schedule magnetometer, timer, and rotary encoder events to monitor box status and user input
            eventRegister(magBoxStatusEvent, EVENT_MAGNOMETER, EVENT_DELTA, 1000, 0);
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
            eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
            break;

        case UNLOCKED_FULL_AWAKE_FUNC_B:
            // similar to func A, with timer and rotary encoder events to monitor status
            eventRegister(magBoxStatusEvent, EVENT_MAGNOMETER, EVENT_DELTA, 1000, 0);
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
            eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
            break;

        case UNLOCKED_FULL_ASLEEP:
            // schedule accelerometer, magnetometer, and rotary encoder events to detect movement or interaction
            eventRegister(accDeltaEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 10, 0);
            eventRegister(magBoxStatusEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 1000, 0);
            eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
            break;

        case UNLOCKED_TO_LOCKED_AWAKE:
            // schedule a timer event to transition after 5 seconds
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, 5000, 0);
            break;

        case LOCKED_FULL_AWAKE:
            // schedule magnetometer and timer events to monitor box status and transition
            eventRegister(magBoxStatusEvent, EVENT_MAGNOMETER, EVENT_DELTA, 1000, 0);
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
            break;

        case LOCKED_FULL_NOTIFICATION_FUNC_A:
            // schedule magnetometer, timer, and rotary encoder events to monitor box status and handle user interaction
            eventRegister(magBoxStatusEvent, EVENT_MAGNOMETER, EVENT_DELTA, 1000, 0);
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
            eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
            break;

        case LOCKED_FULL_NOTIFICATION_FUNC_B:
            // similar to func A but with accelerometer to check box status
            eventRegister(magBoxStatusEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 1000, 0);
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
            eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
            break;

        case LOCKED_FULL_ASLEEP:
            // schedule accelerometer, magnetometer, and rotary encoder events to monitor box movement and user interaction
            eventRegister(accDeltaEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 10, 0);
            eventRegister(magBoxStatusEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 1000, 0);
            eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
            break;

        case LOCKED_MONITOR_AWAKE:
            // schedule events for magnetometer, timer, and audio detection to monitor box status and listen for audio match
            eventRegister(magBoxStatusEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 1000 , 0);
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
            eventRegister(audioEventCallback, EVENT_AUDIO, EVENT_DELTA, 1, 0);
            break;

        case LOCKED_MONITOR_ASLEEP:
            // schedule events for accelerometer, magnetometer, timer, rotary encoder, and audio detection to monitor box status
            eventRegister(accDeltaEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 10, 0);
            eventRegister(magBoxStatusEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 10, 0);
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
            eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
            eventRegister(audioEventCallback, EVENT_AUDIO, EVENT_DELTA, 1, 0);
            break;

        case EMERGENCY_OPEN:
            // schedule timer event to transition after 5 seconds in emergency state
            eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, 5000, 0);
            break;

        default:
            break;
    }
}

#ifdef DEBUG_STATE_CONTROLLER
// converts state flags to string for debugging/logging
const char* SFlagToStr(SFlag flag) {
    switch (flag) {
    case SFLAG_NULL: return "SFLAG_NULL";
    case SFLAG_ACC_BOX_MOVED: return "SFLAG_ACC_BOX_MOVED";
    case SFLAG_ADC_INTERRUPT: return "SFLAG_ADC_INTERRUPT";
    case SFLAG_ADC_MATCH: return "SFLAG_ADC_MATCH";
    case SFLAG_ROTENC_INTERRUPT: return "SFLAG_ROTENC_INTERRUPT";
    case SFLAG_ROTENC_ROTATED: return "SFLAG_ROTENC_ROTATED";
    case SFLAG_TIMER_COMPLETE: return "SFLAG_TIMER_COMPLETE";
    case SFLAG_NFC_PHONE_PRESENT: return "SFLAG_NFC_PHONE_PRESENT";
    case SFLAG_NFC_PHONE_NOT_PRESENT: return "SFLAG_NFC_PHONE_NOT_PRESENT";
    case SFLAG_BOX_CLOSED: return "SFLAG_BOX_CLOSED";
    case SFLAG_BOX_OPEN: return "SFLAG_BOX_OPEN";
    case SFLAG_AUDIO_VOL_HIGH: return "SFLAG_AUDIO_VOL_HIGH";
    case SFLAG_AUDIO_MATCH: return "SFLAG_AUDIO_MATCH";
    case SFLAG_AUDIO_NO_MATCH: return "SFLAG_AUDIO_NO_MATCH";
    default: return "UNKNOWN_SFLAG";
    }
}
#endif
