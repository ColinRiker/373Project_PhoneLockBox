/*
 * state_machine.h
 *
 *  Created on: Mar 28, 2025
 *      Author: colinriker
 */
#include "shared.h"

#ifndef INC_STATE_MACHINE_H_
#define INC_STATE_MACHINE_H_

#include <stdbool.h>

#define MAX_FLAGS 10

typedef enum {
	SFLAG_NULL,
	SFLAG_ACC_BOX_MOVED,
	SFLAG_ADC_INTERRUPT,
	SFLAG_ADC_MATCH,
	SFLAG_ROTENC_INTERRUPT,
	SFLAG_ROTENC_ROTATED,
	SFLAG_TIMER_COMPLETE,
	SFLAG_NFC_PHONE_PRESENT,
	SFLAG_NFC_PHONE_NOT_PRESENT, //proposed addition, because we technically set it low on each state change unless we reinitalize to high?? or will thisbe done
	SFLAG_BOX_CLOSED,
	SFLAG_BOX_OPEN, //consider this lol
	SFLAG_AUDIO_VOL_HIGH, //is audio volume high
	SFLAG_AUDIO_MATCH, //is there an audio match
	SFLAG_AUDIO_NO_MATCH
} SFlag;


void stateMachineInit(void);
BoxMode runStateMachine(void);
bool stateInsertFlag(SFlag flag);
void stateScheduleEvents(BoxMode mode);
void clearFlags(void);
void inturruptControl(BoxMode mode);

#endif
