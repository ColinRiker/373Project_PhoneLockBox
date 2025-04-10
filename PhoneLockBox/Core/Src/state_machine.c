#include "state_machine.h"
#include "shared.h"
#include "rotary_encoder.h"
#include "accelerometer.h"
// #include "microphone.h" // add when ready
#include "nfc.h"        // add when ready
// #include "display.h"    // optional

#include <stdio.h>
#include <stdlib.h>

#define MAX_FLAGS 8

SFlag flags[MAX_FLAGS];

extern BoxState state;
extern BoxState next_state;

extern Vector3D accelerometer_state;
extern Vector3D prev_accelerometer_state;
extern Vector3D magnometer_state;

void stateMachineInit(void) {
	state.mode = UNLOCKED_EMPTY_ASLEEP;
	state.interrupt_flag = NO_INTERRUPT;

	accelerometer_state.x_componenet = 0;
	accelerometer_state.y_componenet = 0;
	accelerometer_state.z_componenet = 0;

	prev_accelerometer_state = accelerometer_state;

	magnometer_state.x_componenet = 0;
	magnometer_state.y_componenet = 0;
	magnometer_state.z_componenet = 0;

	next_state = state;
}

BoxMode runStateMachine(void) {
	BoxMode curr = state.mode;
	BoxMode next = curr;

	switch (curr) {

	case UNLOCKED_EMPTY_ASLEEP:
		// if the box moves, wake up


	case UNLOCKED_ASLEEP_TO_AWAKE:
		// we only move to one other state form this state
		//note that timer resolve is declared but has no current implementation


	case UNLOCKED_EMPTY_AWAKE:
		// if phone is placed in box (NFC detect), switch to full awake mode
		//THIS TAKES PRIORITY OVER GOING BACK TO SLEEP



		break;

	case UNLOCKED_FULL_AWAKE_FUNC_A:
		// PRIORITIZE MOVING TO AWAKE FUNC B

		break;
	case UNLOCKED_FULL_AWAKE_FUNC_B:
		// PRIORITIZE MOVING TO UNLOCKED_TO_LOCKED_AWAKE

		break;
	case UNLOCKED_FULL_ASLEEP:
		// PRIORITIZE MOVING BACK TO EMPTY IF PHONE IS EVER TAKEN OUT

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
