/*
 * state_machine.h
 *
 *  Created on: Mar 28, 2025
 *      Author: colinriker
 */
#include "shared.h"

#ifndef INC_STATE_MACHINE_H_
#define INC_STATE_MACHINE_H_


#define MAX_FLAGS 8

typedef enum {
	SFLAG_NULL,
	SFLAG_ACC_BOX_MOVED,
	SFLAG_ADC_INTERRUPT,
	SFLAG_ADC_MATCH,
	SFLAG_ROTENC_INTERRUPT,
	SFLAG_ROTENC_ROTATED,
	SFLAG_TIMER_COMPLETE,
	SFLAG_NFC_PHONE_PRESENT,

	SFLAG_BOX_OPEN,
	SFLAG_BOX_CLOSED,
	SFLAG_AUDIO_VOL_HIGH, //is audio volume high
	SFLAG_AUDIO_MATCH //is there an audio match
} SFlag;


BoxMode runStateMachine(void);
void stateMachineInit(void);
//THESE NEED TO BE MOVED EVENTUALLY
BoxMode timerResolve(void);
BoxMode NFCResolve(void);
void stateScheduleEvents(BoxMode mode);
void stateTimerCallback(void);

#endif
