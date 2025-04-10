#include "state_machine.h"
#include "shared.h"
#include "rotary_encoder.h"
#include "accelerometer.h"
// #include "microphone.h" // add when ready
#include "nfc.h"        // add when ready
// #include "display.h"    // optional

#include <stdio.h>
#include <stdlib.h>


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
		if (accResolve()==UNLOCKED_ASLEEP_TO_AWAKE) {
		      next = UNLOCKED_ASLEEP_TO_AWAKE;
		}
		break;

	case UNLOCKED_ASLEEP_TO_AWAKE:
		// show hello msg / animate screen on
		// we only move to one other state form this state
		//note that timer resolve is declared but has no current implementation
		if(timerResolve()==UNLOCKED_EMPTY_AWAKE) {
			next = UNLOCKED_EMPTY_AWAKE;

		}
		break;

	case UNLOCKED_EMPTY_AWAKE:
		//THIS TAKES PRIORITY OVER GOING BACK TO SLEEP
		if (NFCResolve()==UNLOCKED_FULL_AWAKE_FUNC_A) {
			next = UNLOCKED_FULL_AWAKE_FUNC_A;
		}
		//if we cannot move to full awake, we move back to sleep
		else if((rotencResolve()==UNLOCKED_EMPTY_ASLEEP) && (timerResolve()==UNLOCKED_EMPTY_ASLEEP)) {
			next = UNLOCKED_EMPTY_ASLEEP;
		}
		// if user does nothing for a while, go back to sleep
		// TODO: add timer / timeout
		break;

	case UNLOCKED_FULL_AWAKE_FUNC_A:
		//who wrote that lol not sure what to do with this line
		 if (/* isButtonHeldLong() */ 0) {
			 next = UNLOCKED_TO_LOCKED_AWAKE;
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
		 }// if not, wake back up if possible
		 else if(rotencResolve()==UNLOCKED_FULL_AWAKE_FUNC_A && accResolve()==UNLOCKED_FULL_AWAKE_FUNC_A) {
			 next=UNLOCKED_FULL_AWAKE_FUNC_A;
		 }
		break;
	case UNLOCKED_TO_LOCKED_AWAKE:
		// if the button is pressed but there is NO phone at any point
		// prioritize this
		if(accResolve()==UNLOCKED_FULL_AWAKE_FUNC_B && rotencResolve()==UNLOCKED_FULL_AWAKE_FUNC_B) {
			next = UNLOCKED_FULL_AWAKE_FUNC_B;
		} // if the button is pressed, thre IS a phone and it has been 5 seconds
		else if(timerResolve()==LOCKED_FULL_AWAKE && accResolve()==LOCKED_FULL_AWAKE && rotencResolve()==LOCKED_FULL_AWAKE) {
			next = LOCKED_FULL_AWAKE;
		}
		break;

	case LOCKED_FULL_AWAKE:
		//if at any point phone not in box, enter emergency state
		if(accResolve()==EMERGENCY_OPEN) {
			next = EMERGENCY_OPEN;
		}
		// this is the main countdown state
		if (state.time > 0) {
			state.time--;
		} else {
			// unlock and go back to unlocked state
			next = UNLOCKED_EMPTY_AWAKE;
		}

		// optional: monitor for emergency unlock here
		break;

	case LOCKED_FULL_NOTIFICATION_FUNC_A:
		//if at any point phone not in box, enter emergency state
		if(accResolve()==EMERGENCY_OPEN) {
			next = EMERGENCY_OPEN;
		}
		//if we hold button in this state, unlock
		else if(rotarResolve()==UNLOCKED_FULL_AWAKE_FUNC_B) {
			next = UNLOCKED_FULL_AWAKE_FUNC_B;
		} //if we MOVE dial in any way, then we are changing selection
		else if(rotarResolve()=LOCKED_FULL_NOTIFICATION_FUNC_B) {
			next = LOCKED_FULL_NOTIFICATION_FUNC_B;
		} // if it's been a minute, assume they are choosing to ignore and go bck to locked (but awake)
		else if(timerResolve()==LOCKED_FULL_AWAKE) {
			next=LOCKED_FULL_AWAKE;
		}
		break;
	case LOCKED_FULL_NOTIFICATION_FUNC_B:
		//if at any point phone not in box, enter emergency state
		if(accResolve()==EMERGENCY_OPEN) {
			next = EMERGENCY_OPEN;
		}
		else if(rotarResolve()=LOCKED_FULL_NOTIFICATION_FUNC_A) {
			next = LOCKED_FULL_NOTIFICATION_FUNC_A;
		} // if it's been a minute, assume they are choosing to ignore and go bck to locked (but awake)
		else if(timerResolve()==LOCKED_FULL_AWAKE) {
			next=LOCKED_FULL_AWAKE;
		}

		break;
	case LOCKED_FULL_ASLEEP:
		//if at any point phone not in box, enter emergency state
		if(accResolve()==EMERGENCY_OPEN) {
			next = EMERGENCY_OPEN;
		}
		else if() {

		}
		else if() {

		}
		break;
	case LOCKED_MONITOR_AWAKE:
		//if at any point phone not in box, enter emergency state
		if(accResolve()==EMERGENCY_OPEN) {
			next = EMERGENCY_OPEN;
		}
		else if() {

		}
		else if() {

		}
		break;
	case LOCKED_MONITOR_ASLEEP:
		//if at any point phone not in box, enter emergency state
		if(accResolve()==EMERGENCY_OPEN) {
			next = EMERGENCY_OPEN;
		}
		else if() {

		}
		else if() {

		}
		break;
	case EMERGENCY_OPEN:
		//if we were in this for 5 seconds, move to UNLOCKED_FULL_AWAKE_FUNC_B
		if(timerResolve()==UNLOCKED_FULL_AWAKE_FUNC_B) {
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

	return state.mode;
}
