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
		// we only move to one other state form this state
		//note that timer resolve is declared but has no current implementation
		if(timerResolve()==UNLOCKED_EMPTY_AWAKE) {
			next = UNLOCKED_EMPTY_AWAKE;
		}
		break;

	case UNLOCKED_EMPTY_AWAKE:
		// if phone is placed in box (NFC detect), switch to full awake mode
		//THIS TAKES PRIORITY OVER GOING BACK TO SLEEP
		if (NFCResolve()==UNLOCKED_FULL_AWAKE_FUNC_A) {
			next = UNLOCKED_FULL_AWAKE_FUNC_A;
		}
		//if we cannot move to full awake, we move back to sleep
		else if((rotencResolve()==UNLOCKED_EMPTY_ASLEEP) && (timerResolve()==UNLOCKED_EMPTY_ASLEEP)) {
			next = UNLOCKED_EMPTY_ASLEEP;
		}


		break;

	case UNLOCKED_FULL_AWAKE_FUNC_A:
		// PRIORITIZE MOVING TO AWAKE FUNC B
		if (rotencResolve()==UNLOCKED_FULL_AWAKE_FUNC_B) {
			next=UNLOCKED_FULL_AWAKE_FUNC_B;
		} //if we can't move to taht, we move back to sleep
		else if(timerResolve()==UNLOCKED_FULL_ASLEEP) {
			next=UNLOCKED_FULL_ASLEEP;
		}
		break;
	case UNLOCKED_FULL_AWAKE_FUNC_B:
		// PRIORITIZE MOVING TO UNLOCKED_TO_LOCKED_AWAKE
		if (rotencResolve()==UNLOCKED_TO_LOCKED_AWAKE && accResolve()==UNLOCKED_TO_LOCKED_AWAKE) {
			next=UNLOCKED_TO_LOCKED_AWAKE;
		} //if we can't move to UNLOCKED_TO_LOCKED_AWAKE, we move to func A
		else if(rotencResolve()==UNLOCKED_FULL_AWAKE_FUNC_A) {
			next=UNLOCKED_FULL_ASLEEP;
		}  //next, move back to unlocked and empty but awake
		else if(NFCResolve()==UNLOCKED_EMPTY_AWAKE) {
			next=UNLOCKED_EMPTY_AWAKE;
		} //finally, we last move back to sleep
		else if(timerResolve()==UNLOCKED_FULL_ASLEEP) {
			next=UNLOCKED_FULL_ASLEEP;
		}
		break;
	case UNLOCKED_FULL_ASLEEP:
		// PRIORITIZE MOVING BACK TO EMPTY IF PHONE IS EVER TAKEN OUT
		if (NFCResolve()==UNLOCKED_EMPTY_AWAKE) {
			next=UNLOCKED_EMPTY_AWAKE;
		}//if not, wake back up if possible
		else if(rotencResolve()==UNLOCKED_FULL_AWAKE_FUNC_A && accResolve()==UNLOCKED_FULL_AWAKE_FUNC_A) {
			next=UNLOCKED_FULL_AWAKE_FUNC_A;
		}
		break;
	case UNLOCKED_TO_LOCKED_AWAKE:

		break;

	case LOCKED_FULL_AWAKE:

		break;

	case LOCKED_FULL_NOTIFICATION_FUNC_A:

		break;
	case LOCKED_FULL_NOTIFICATION_FUNC_B:

		break;
	case LOCKED_FULL_ASLEEP:

		break;
	case LOCKED_MONITOR_AWAKE:

		break;
	case LOCKED_MONITOR_ASLEEP:

		break;


	default: //should we have a default?? what would that be??
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
