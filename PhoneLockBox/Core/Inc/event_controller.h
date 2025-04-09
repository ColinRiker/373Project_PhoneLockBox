/*
 * event_controller.h
 *
 *  Created on: Mar 28, 2025
 *      Author: colinriker
 */

#include <stdint.h>

#include "shared.h"


#ifndef INC_EVENT_CONTROLLER_H_
#define INC_EVENT_CONTROLLER_H_

#define MAX_EVENT_COUNT 10

typedef enum {
	EVENT_SUCCESS,
	EVENT_LABEL_NOT_FOUND,
	EVENT_LABEL_ALREADY_USED,
	EVENT_INIT_FAILED
} EventReturnCode;

typedef enum {
	EVENT_EMPTY,
	EVENT_NFC,
	EVENT_ROTARY_ENCODER,
	EVENT_AUDIO,
	EVENT_TIMER
} EventLabel;

typedef enum {
	EVENT_DISABLED,
	EVENT_SINGLE,
	EVENT_DELTA
} EventFlag;

typedef struct {
	void (*callback) (void);
	EventLabel label;
	EventFlag flag;
	uint16_t schedule_time;
	uint16_t period;
} Event;

EventReturnCode eventRegister(void *callback, EventLabel label, EventFlag flag, uint16_t period);
EventReturnCode eventRemove(EventLabel label);
EventReturnCode eventUpdate(EventLabel label, EventFlag new_flag, uint16_t new_period);
EventReturnCode eventControllerInit(void);
void eventLoop(void);
void eventDefaultCallback(void);

//Debug Functions
void eventLabelToStr(char* buffer);
void eventFlagToStr(char* buffer);

#endif /*EVENT_CONTROLLER*/
