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
		if (SFLAG_ACC_BOX_MOVED || SFLAG_ROTENC_INTERRUPT || SFLAG_ROTENC_ROTATED) {
		      next = UNLOCKED_ASLEEP_TO_AWAKE;
		}
		break;

	case UNLOCKED_ASLEEP_TO_AWAKE:
		// show hello msg / animate screen on
		// we only move to one other state form this state
		//note that timer resolve is declared but has no current implementation
		if(SFLAG_TIMER_COMPLETE) {
			next = UNLOCKED_EMPTY_AWAKE;
		}
		break;

	case UNLOCKED_EMPTY_AWAKE:
		//THIS TAKES PRIORITY OVER GOING BACK TO SLEEP
		if (SFLAG_NFC_PHONE_PRESENT) {
			next = UNLOCKED_FULL_AWAKE_FUNC_A;
		}
		//if we cannot move to full awake, we move back to sleep
		else if(SFLAG_TIMER_COMPLETE && SFLAG_ROTENC_INTERRUPT) {
			next = UNLOCKED_EMPTY_ASLEEP;
		}

		break;

	case UNLOCKED_FULL_AWAKE_FUNC_A:
		 // PRIORITIZE MOVING TO AWAKE FUNC B
		 if (SFLAG_ROTENC_ROTATED) {
			 next=UNLOCKED_FULL_AWAKE_FUNC_B;
		 } //if we can't move to that, we move back to sleep
		 else if(SFLAG_TIMER_COMPLETE) {
			 next=UNLOCKED_FULL_ASLEEP;
		 }
		break;
	case UNLOCKED_FULL_AWAKE_FUNC_B:
		// PRIORITIZE MOVING TO UNLOCKED_TO_LOCKED_AWAKE
		if (SFLAG_ROTENC_INTERRUPT && SFLAG_BOX_CLOSED) {
			next=UNLOCKED_TO_LOCKED_AWAKE;
		} //if we can't move to UNLOCKED_TO_LOCKED_AWAKE, we move to func A
		else if(SFLAG_ROTENC_ROTATED) {
			next=UNLOCKED_FULL_AWAKE_FUNC_A;
		}  //next, move back to unlocked and empty but awake
		else if(!SFLAG_NFC_PHONE_PRESENT) {
			next=UNLOCKED_EMPTY_AWAKE;
		} //finally, we last move back to sleep
		else if(SFLAG_TIMER_COMPLETE) {
			next=UNLOCKED_FULL_ASLEEP;
		}
		break;
	case UNLOCKED_FULL_ASLEEP:
		 // PRIORITIZE MOVING BACK TO EMPTY IF PHONE IS EVER TAKEN OUT
		 if (!SFLAG_NFC_PHONE_PRESENT) {
			 next=UNLOCKED_EMPTY_AWAKE;
		 }// if not, wake back up if possible
		 else if(SFLAG_ACC_BOX_MOVED || SFLAG_ROTENC_ROTATED || SFLAG_ROTENC_INTERRUPT) {
			 next=UNLOCKED_FULL_AWAKE_FUNC_A;
		 }
		break;
	case UNLOCKED_TO_LOCKED_AWAKE:
		// if the button is pressed OR there is NO phone at any point OR box is opened
		// prioritize this
		if((!SFLAG_NFC_PHONE_PRESENT) || SFLAG_BOX_OPEN || SFLAG_ROTENC_INTERRUPT) {
			next = UNLOCKED_FULL_AWAKE_FUNC_B;
		} // if it has been 5 seconds in this state
		else if(SFLAG_TIMER_COMPLETE) {
			next = LOCKED_FULL_AWAKE;
		}
		break;

	case LOCKED_FULL_AWAKE:
		//if at any point phone not in box, enter emergency state
		if(SFLAG_BOX_OPEN) {
			next = EMERGENCY_OPEN;
		}
		//sleep if timer hits 1 min
		else if(SFLAG_TIMER_COMPLETE) {
			next = LOCKED_FULL_ASLEEP;
		}
		//start monitoring audio
		else if(SFLAG_AUDIO_VOL_HIGH) {
			next = LOCKED_MONITOR_AWAKE;
		}

		// optional: monitor for emergency unlock here
		break;

	case LOCKED_FULL_NOTIFICATION_FUNC_A:
		//if at any point phone not in box, enter emergency state
		if(SFLAG_BOX_OPEN) {
			next = EMERGENCY_OPEN;
		}
		//if we hold button in this state, unlock
		else if(SFLAG_ROTENC_INTERRUPT) {
			next = UNLOCKED_FULL_AWAKE_FUNC_B;
		} //if we MOVE dial in any way, then we are changing selection
		else if(SFLAG_ROTENC_ROTATED) {
			next = LOCKED_FULL_NOTIFICATION_FUNC_B;
		} // if it's been a minute, assume they are choosing to ignore and go bck to locked (but awake)
		else if(SFLAG_TIMER_COMPLETE) {
			next=LOCKED_FULL_AWAKE;
		}
		break;
	case LOCKED_FULL_NOTIFICATION_FUNC_B:
		//if at any point phone not in box, enter emergency state
		if(SFLAG_BOX_OPEN) {
			next = EMERGENCY_OPEN;
		}
		//
		else if(SFLAG_ROTENC_ROTATED) {
			next = LOCKED_FULL_NOTIFICATION_FUNC_A;
		} // if it's been a minute, assume they are choosing to ignore and go bck to locked (but awake)
		else if(SFLAG_TIMER_COMPLETE) {
			next=LOCKED_FULL_AWAKE;
		}
		break;
	case LOCKED_FULL_ASLEEP:
		//if at any point phone not in box, enter emergency state
		if(SFLAG_BOX_OPEN) {
			next = EMERGENCY_OPEN;
		}
		//if volume is high, we do not wake up monitor but we do listen to sound
		else if(SFLAG_AUDIO_VOL_HIGH) {
			next = LOCKED_MONITOR_ASLEEP;
		}
		//if button pressed OR dial changed OR box moved, wake up
		else if(SFLAG_ROTENC_INTERRUPT || SFLAG_ROTENC_ROTATED || SFLAG_ACC_BOX_MOVED) {
			next = LOCKED_FULL_AWAKE;
		}
		break;
	case LOCKED_MONITOR_AWAKE:
		//if at any point phone not in box, enter emergency state
		if(SFLAG_BOX_OPEN) {
			next = EMERGENCY_OPEN;
		}
		//if there is a match, we move to prompt user if they want to unlock
		else if(SFLAG_AUDIO_MATCH) {
			next = LOCKED_FULL_NOTIFICATION_FUNC_B;
		} else { //otherwise (if no match) we move back to locked state
			next = LOCKED_FULL_AWAKE;
		}

		break;
	case LOCKED_MONITOR_ASLEEP:
		//if at any point phone not in box, enter emergency state
		if(SFLAG_BOX_OPEN) {
			next = EMERGENCY_OPEN;
		}
		//if there is an audio match, prompt user
		else if(SFLAG_AUDIO_MATCH) {
			next = LOCKED_FULL_NOTIFCATION_B;
		}
		//if there was no audio match, go back to sleep
		else  {
			next = LOCKED_FULL_ASLEEP;
		}
		break;
	case EMERGENCY_OPEN:
		//if we were in this for 5 seconds, move to UNLOCKED_FULL_AWAKE_FUNC_B
		if(SFLAG_TIMER_COMPLETE) {
			next=UNLOCKED_FULL_AWAKE_FUNC_B
		}
		break;
	default: //should we have a default?? what would that be??
			// fallback case
		break;
	}

	// update state if changed
	if (next != curr) {
		eventClear();
		printf("transition: %d → %d\n", curr, next);
		state.mode = next;
		next_state.mode = next;
	}

	}

