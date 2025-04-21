/*
 * State Machine
 * Handles the various transitions and functions of the box
 *
 * */

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

#define MINUTE 60000

BoxState previous;
BoxState state;
SFlag flags[MAX_FLAGS];
bool master_timer_done; //Master time is the lock timer

bool hasFlag(SFlag flag) {
	for (uint8_t i = 0; i < MAX_FLAGS; ++i) {
		if (flags[i] == flag) return true;
	}
	return false;
}

bool stateInsertFlag(SFlag flag) {
#ifdef DEBUG_STATE_CONTROLLER
	printf("[INFO] Inserting State flag: %s\n\r", SFlagToStr(flag));
#endif

	uint8_t empty_idx = MAX_FLAGS;
	for(uint8_t i = 0; i < MAX_FLAGS; ++i) {
		if (flags[i] == flag) {
			return false;
		} else if (flags[i] == SFLAG_NULL && i < empty_idx) {
			empty_idx = i;
		}
	}

	if (empty_idx < MAX_FLAGS) {
		flags[empty_idx] = flag;
		return true;
	}

	return false;

}

void clearFlags(void) {
	for (uint8_t i = 0; i < MAX_FLAGS; ++i) {
		flags[i] = SFLAG_NULL;
	}
}

void inturruptControl(void) {

	switch(state) {
	//all states that need to have button DISABLED
	case UNLOCKED_ASLEEP_TO_AWAKE:
	case UNLOCKED_FULL_AWAKE_FUNC_A:
	case LOCKED_FULL_AWAKE:
	case LOCKED_MONITOR_AWAKE:
	case LOCKED_MONITOR_ASLEEP:
	case EMERGENCY_OPEN:
		//func to DISABLE button interrupt
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);  // button
		break;
		//all states that need to have button ENABLED
	case UNLOCKED_EMPTY_ASLEEP:
	case UNLOCKED_EMPTY_AWAKE:
	case UNLOCKED_FULL_ASLEEP:
	case UNLOCKED_FULL_AWAKE_FUNC_B:
	case UNLOCKED_TO_LOCKED_AWAKE:
	case LOCKED_FULL_NOTIFICATION_FUNC_A:
	case LOCKED_FULL_NOTIFICATION_FUNC_B:
	case LOCKED_FULL_ASLEEP:
		//func to ENABLE button interrupt
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
		break;
	}

	switch(state) {
	//all states that need to have audio DISABLED
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
		//func to DISABLE audio interrupt
		HAL_NVIC_DisableIRQ(EXTI0_IRQn);      // audio
		break;
		//all states that need to have audio ENABLED
	case LOCKED_MONITOR_AWAKE:
	case LOCKED_FULL_ASLEEP:
	case LOCKED_FULL_AWAKE:
	case LOCKED_MONITOR_ASLEEP:
		//func to ENABLE audio interrupt
		HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		break;

	}
}


void stateMachineInit(void) {
	state = UNLOCKED_EMPTY_ASLEEP;
}

void runStateMachine(void) {
	BoxState next = state;

	switch (state) {

	case UNLOCKED_EMPTY_ASLEEP:
		// if the box moves, wake up
		if (hasFlag(SFLAG_ACC_BOX_MOVED) || hasFlag(SFLAG_ROTENC_INTERRUPT) || hasFlag(SFLAG_ROTENC_ROTATED)) {
			next = UNLOCKED_ASLEEP_TO_AWAKE;
		}
		break;

	case UNLOCKED_ASLEEP_TO_AWAKE:
		// show hello msg / animate screen on
		// we only move to one other state form this state
		//note that timer resolve is declared but has no current implementation
		if(hasFlag(SFLAG_TIMER_COMPLETE)) {
			next = UNLOCKED_EMPTY_AWAKE;
		}
		break;

	case UNLOCKED_EMPTY_AWAKE:
		//THIS TAKES PRIORITY OVER GOING BACK TO SLEEP
		if (hasFlag(SFLAG_NFC_PHONE_PRESENT)) {
			next = UNLOCKED_FULL_AWAKE_FUNC_A;
		}
		//if we cannot move to full awake, we move back to sleep
		else if(hasFlag(SFLAG_TIMER_COMPLETE) || hasFlag(SFLAG_ROTENC_INTERRUPT)) {
			next = UNLOCKED_EMPTY_ASLEEP;
		}

		break;

	case UNLOCKED_FULL_AWAKE_FUNC_A:
		//if we take out phone at ANY point, we gotta go back to UNLCOKED_EMPTY_AWAKE
		if(hasFlag(SFLAG_NFC_PHONE_NOT_PRESENT)) {
			next=UNLOCKED_EMPTY_AWAKE;
		}
		// PRIORITIZE MOVING TO AWAKE FUNC B
		else if (hasFlag(SFLAG_ROTENC_ROTATED)) {
			next=UNLOCKED_FULL_AWAKE_FUNC_B;
		} //otherwise we are just charging!
		else if(hasFlag(SFLAG_TIMER_COMPLETE) || hasFlag(SFLAG_ROTENC_INTERRUPT)) {
			next=UNLOCKED_FULL_ASLEEP;
		}
		break;
	case UNLOCKED_FULL_AWAKE_FUNC_B:
		// PRIORITIZE MOVING TO UNLOCKED_TO_LOCKED_AWAKE
		if (hasFlag(SFLAG_ROTENC_INTERRUPT) && hasFlag(SFLAG_BOX_CLOSED)) {
			next=UNLOCKED_TO_LOCKED_AWAKE;
		} //if we can't move to UNLOCKED_TO_LOCKED_AWAKE, we move to func A
		else if(hasFlag(SFLAG_ROTENC_ROTATED)) {
			next=UNLOCKED_FULL_AWAKE_FUNC_A;
		}  //next, move back to unlocked and empty but awake
		else if(hasFlag(SFLAG_NFC_PHONE_NOT_PRESENT)) {
			next=UNLOCKED_EMPTY_AWAKE;
		} //finally, we last move back to sleep
		else if(hasFlag(SFLAG_TIMER_COMPLETE)) {
			next=UNLOCKED_FULL_ASLEEP;
		}
		break;
	case UNLOCKED_FULL_ASLEEP:
		// PRIORITIZE MOVING BACK TO EMPTY IF PHONE IS EVER TAKEN OUT
		if (hasFlag(SFLAG_NFC_PHONE_NOT_PRESENT)) {
			next=UNLOCKED_EMPTY_AWAKE;
		}// if not, wake back up if possible
		else if(hasFlag(SFLAG_ACC_BOX_MOVED) || hasFlag(SFLAG_ROTENC_ROTATED) || hasFlag(SFLAG_ROTENC_INTERRUPT)) {
			next=UNLOCKED_FULL_AWAKE_FUNC_A;
		}
		break;
	case UNLOCKED_TO_LOCKED_AWAKE:
		// if the button is pressed OR there is NO phone at any point OR box is opened
		// prioritize this
		if((hasFlag(SFLAG_NFC_PHONE_NOT_PRESENT)) || hasFlag(SFLAG_BOX_OPEN) || hasFlag(SFLAG_ROTENC_INTERRUPT)) {
			next = UNLOCKED_FULL_AWAKE_FUNC_B;
		} // if it has been 5 seconds in this state
		else if(hasFlag(SFLAG_TIMER_COMPLETE)) {
			next = LOCKED_FULL_AWAKE;
		}
		break;

	case LOCKED_FULL_AWAKE:
		//if at any point phone not in box, enter emergency state
		if(hasFlag(SFLAG_BOX_OPEN)) {
			next = EMERGENCY_OPEN;
		}
		//sleep if timer hits 1 min
		else if(hasFlag(SFLAG_TIMER_COMPLETE)) {
			next = LOCKED_FULL_ASLEEP;
		}
		//start monitoring audio
		else if(hasFlag(SFLAG_AUDIO_VOL_HIGH)) {
			next = LOCKED_MONITOR_AWAKE;
		}

		// optional: monitor for emergency unlock here
		break;

	case LOCKED_FULL_NOTIFICATION_FUNC_A:
		//if at any point phone not in box, enter emergency state
		if(hasFlag(SFLAG_BOX_OPEN)) {
			next = EMERGENCY_OPEN;
		}
		//if we hold button in this state, unlock
		else if(hasFlag(SFLAG_ROTENC_INTERRUPT)) {
			next = UNLOCKED_FULL_AWAKE_FUNC_B;
		} //if we MOVE dial in any way, then we are changing selection
		else if(hasFlag(SFLAG_ROTENC_ROTATED)) {
			next = LOCKED_FULL_NOTIFICATION_FUNC_B;
		} // if it's been a minute, assume they are choosing to ignore and go bck to locked (but awake)
		else if(hasFlag(SFLAG_TIMER_COMPLETE)) {
			next=LOCKED_FULL_AWAKE;
		}
		break;
	case LOCKED_FULL_NOTIFICATION_FUNC_B:
		//if at any point phone not in box, enter emergency state
		if(hasFlag(SFLAG_BOX_OPEN)) {
			next = EMERGENCY_OPEN;
		}
		//
		else if(hasFlag(SFLAG_ROTENC_ROTATED)) {
			next = LOCKED_FULL_NOTIFICATION_FUNC_A;
		} // if it's been a minute, assume they are choosing to ignore and go bck to locked (but awake)
		else if(hasFlag(SFLAG_TIMER_COMPLETE)) {
			next=LOCKED_FULL_AWAKE;
		}else if (hasFlag(SFLAG_ROTENC_INTERRUPT)){
			next=LOCKED_FULL_AWAKE;
		}
		break;
	case LOCKED_FULL_ASLEEP:
		//if at any point phone not in box, enter emergency state
		if(hasFlag(SFLAG_BOX_OPEN)) {
			next = EMERGENCY_OPEN;
		}
		//if volume is high, we do not wake up monitor but we do listen to sound
		else if(hasFlag(SFLAG_AUDIO_VOL_HIGH)) {
			next = LOCKED_MONITOR_ASLEEP;
		}
		//if button pressed OR dial changed OR box moved, wake up
		else if(hasFlag(SFLAG_ROTENC_INTERRUPT) || hasFlag(SFLAG_ROTENC_ROTATED) || hasFlag(SFLAG_ACC_BOX_MOVED)) {
			next = LOCKED_FULL_AWAKE;
		}
		break;
	case LOCKED_MONITOR_AWAKE:
		//if at any point phone not in box, enter emergency state
		if(hasFlag(SFLAG_BOX_OPEN)) {
			next = EMERGENCY_OPEN;
		}
		//if there is a match, we move to prompt user if they want to unlock
		else if(hasFlag(SFLAG_AUDIO_MATCH)) {
			next = LOCKED_FULL_NOTIFICATION_FUNC_B;
		} else if(hasFlag(SFLAG_AUDIO_NO_MATCH)){ //otherwise (if no match) we move back to locked state
			next = LOCKED_FULL_AWAKE;
		}

		break;
	case LOCKED_MONITOR_ASLEEP:
		//if at any point phone not in box, enter emergency state
		if(hasFlag(SFLAG_BOX_OPEN)) {
			next = EMERGENCY_OPEN;
		}
		//if there is an audio match, prompt user
		else if(hasFlag(SFLAG_AUDIO_MATCH)) {
			next = LOCKED_FULL_NOTIFICATION_FUNC_B;
		}
		//if there was no audio match, go back to sleep
		else if(hasFlag(SFLAG_AUDIO_NO_MATCH)) {
			next = LOCKED_FULL_ASLEEP;
		}
		break;
	case EMERGENCY_OPEN:
		//if we were in this for 5 seconds, move to UNLOCKED_FULL_AWAKE_FUNC_B
		if(hasFlag(SFLAG_TIMER_COMPLETE)) {
			next=UNLOCKED_FULL_AWAKE_FUNC_B;
		}
		break;
	default:
#ifdef DEBUG_STATE_CONTROLLER
		printf("[ERROR] Default case of state machine reached, system in bad state\n\r");
#endif
		break;
	}

	if(master_timer_done) { //if we trigger overall timer
		next = UNLOCKED_FULL_AWAKE_FUNC_A;
		//reset
		master_timer_done = false;
	}

	// update state if changed
	if (next != state) {
		/* State Cleanup*/
		eventClear();
		clearFlags();

#ifdef DEBUG_STATE_CONTROLLER
		printf("\n[Info] --- State transition: %s → %s ---\n\r", stateToStr(state), stateToStr(next));
#endif

		stateTransitionCleanup(next);

		previous = state;
		state = next;

		/* State Setup */
		inturruptControl();
		stateScheduleEvents();
		screenResolve();
	}

}

void stateTransitionCleanup (BoxState next) {
	//Transition Specific Logic
	if (state == UNLOCKED_TO_LOCKED_AWAKE && next == LOCKED_FULL_AWAKE) {
		lockTimerStart();
		lockEngage();
	} else if (( state == LOCKED_FULL_AWAKE || state == LOCKED_FULL_ASLEEP) && next == UNLOCKED_FULL_AWAKE_FUNC_B) {
		lockTimerCancel();
		lockDisenage();
	} else if (next == EMERGENCY_OPEN) {
		lockTimerCancel();
		lockDisenage();
	}
}


void stateScheduleEvents() {
	switch (state) {
	case UNLOCKED_EMPTY_ASLEEP:
		eventRegister(accDeltaEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 10, 0);
		eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
		break;

	case UNLOCKED_ASLEEP_TO_AWAKE:
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, 1000, 0);
		break;

	case UNLOCKED_EMPTY_AWAKE:
		eventRegister(nfcEventCallbackSlow, EVENT_NFC_READ, EVENT_DELTA, 1000, 0);
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
		break;

	case UNLOCKED_FULL_AWAKE_FUNC_A:
		eventRegister(magBoxStatusEvent, EVENT_MAGNOMETER, EVENT_DELTA, 1000, 0);
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
		eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
		break;

	case UNLOCKED_FULL_AWAKE_FUNC_B:
		eventRegister(magBoxStatusEvent, EVENT_MAGNOMETER, EVENT_DELTA, 1000, 0);
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
		eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
		break;

	case UNLOCKED_FULL_ASLEEP:
		eventRegister(accDeltaEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 10, 0);
		eventRegister(magBoxStatusEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 1000, 0);
		eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
		break;

	case UNLOCKED_TO_LOCKED_AWAKE:
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, 5000, 0);
		break;

	case LOCKED_FULL_AWAKE:
		eventRegister(magBoxStatusEvent, EVENT_MAGNOMETER, EVENT_DELTA, 1000, 0);
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
		break;

	case LOCKED_FULL_NOTIFICATION_FUNC_A:
		eventRegister(magBoxStatusEvent, EVENT_MAGNOMETER, EVENT_DELTA, 1000, 0);
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
		eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
		break;

	case LOCKED_FULL_NOTIFICATION_FUNC_B:
		eventRegister(magBoxStatusEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 1000, 0);
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
		eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
		break;

	case LOCKED_FULL_ASLEEP:
		eventRegister(accDeltaEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 10, 0);
		eventRegister(magBoxStatusEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 1000, 0);
		eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
		break;

	case LOCKED_MONITOR_AWAKE:
		eventRegister(magBoxStatusEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 1000 , 0);
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
		eventRegister(audioEventCallback, EVENT_AUDIO, EVENT_DELTA, 1, 0);
		break;

	case LOCKED_MONITOR_ASLEEP:
		eventRegister(accDeltaEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 10, 0);
		eventRegister(magBoxStatusEvent, EVENT_ACCELEROMETER, EVENT_DELTA, 10, 0);
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, MINUTE, 0);
		eventRegister(rotencDeltaEvent, EVENT_ROTARY_ENCODER, EVENT_DELTA, 1, 0);
		eventRegister(audioEventCallback, EVENT_AUDIO, EVENT_DELTA, 1, 0);
		break;
	case EMERGENCY_OPEN:
		eventRegister(eventTimerCallback, EVENT_TIMER, EVENT_SINGLE, 5000, 0);
		break;

	default:
		break;
	}
}

#ifdef DEBUG_STATE_CONTROLLER
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

