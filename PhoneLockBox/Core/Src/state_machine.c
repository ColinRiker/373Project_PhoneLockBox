#include "state_machine.h"
#include "shared.h"
#include "rotary_encoder.h"
#include "accelerometer.h"
// #include "microphone.h" // add when ready
// #include "nfc.h"        // add when ready
// #include "display.h"    // optional

#include <stdio.h>
#include <stdlib.h>


extern BoxState state;
extern BoxState next_state;

void stateMachineInit(void) {
    state.mode = UNLOCKED_EMPTY_ASLEEP;
    state.time = 0;
    next_state = state;
}

BoxMode runStateMachine(void) {
    BoxMode curr = state.mode;
    BoxMode next = curr;

    // you can read inputs from here or call resolve functions
    int16_t x_acc, y_acc, z_acc;
    accRead(&x_acc, &y_acc, &z_acc);
    uint32_t encoder_val = TIM1->CNT; // quick poll for encoder

    switch (curr) {

        case UNLOCKED_EMPTY_ASLEEP:
            // if the box moves, wake up
            if (abs(x_acc) > 2000 || abs(y_acc) > 2000 || abs(z_acc) > 2000) {
                next = UNLOCKED_ASLEEP_TO_AWAKE;
            }
            break;

        case UNLOCKED_ASLEEP_TO_AWAKE:
            // show hello msg / animate screen on
            // after that, move to awake
            next = UNLOCKED_EMPTY_AWAKE;
            break;

        case UNLOCKED_EMPTY_AWAKE:
            // if phone is placed in box (NFC detect), switch to full awake mode
            // just placeholder logic for now
            if (/* phonePresent() */ 0) {
                next = UNLOCKED_FULL_AWAKE_FUNC_A;
            }

            // if user does nothing for a while, go back to sleep
            // TODO: add timer / timeout
            break;

        case UNLOCKED_FULL_AWAKE_FUNC_A:
            // user can turn dial to set time
            // pushbutton long press to lock
            if (encoder_val > 0) {
                state.time = encoder_val;
            }

            if (/* isButtonHeldLong() */ 0) {
                next = UNLOCKED_TO_LOCKED_AWAKE;
            }
            break;

        case UNLOCKED_TO_LOCKED_AWAKE:
            // show setting time animation
            // lock solenoid here
            // transition to locked full awake
            next = LOCKED_FULL_AWAKE;
            break;

        case LOCKED_FULL_AWAKE:
            // this is the main countdown state
            if (state.time > 0) {
                state.time--;
            } else {
                // unlock and go back to unlocked state
                next = UNLOCKED_EMPTY_AWAKE;
            }

            // optional: monitor for emergency unlock here
            break;

        case EMERGENCY_OPEN:
            // user held emergency button, override lock
            // unlock solenoid + return to awake state
            next = UNLOCKED_EMPTY_AWAKE;
            break;

        default:
            // fallback case
            break;
    }

    // update state if changed
    if (next != curr) {
        printf("transition: %d → %d\n", curr, next);
        state.mode = next;
        next_state.mode = next;

    }

    return state.mode;
}
