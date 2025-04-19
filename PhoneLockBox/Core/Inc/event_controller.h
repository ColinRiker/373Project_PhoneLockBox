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

#define MAX_EVENT_COUNT 16 /* MUST BE LESS THAN 256 */
#define MAX_TIME 0xFFFFFFFF

typedef enum {
	EVENT_SUCCESS,
	EVENT_LABEL_NOT_FOUND,
	EVENT_LABEL_ALREADY_USED,
	EVENT_INIT_FAILED,
	EVENT_QUEUE_FULL,
	EVENT_GENERIC_ERROR //Lowk evil to have this but I am lazy
} EventReturnCode;

typedef enum {
	EVENT_EMPTY,
	EVENT_NFC_START_READ,
	EVENT_NFC_POLL,
	EVENT_NFC_READ,
	EVENT_ROTARY_ENCODER,
	EVENT_AUDIO,
	EVENT_TIMER,
	EVENT_ACCELEROMETER,
	EVENT_MAGNOMETER,
} EventLabel;

typedef enum {
	EVENT_DISABLED,
	EVENT_SINGLE,
	EVENT_SINGLE_IMMEDIATE,
	EVENT_DELTA,
	EVENT_DELTA_IMMEDIATE,
	EVENT_N_REPEAT,
	EVENT_N_REPEAT_IMMEDIATE
} EventFlag;

/* EVENT STRUCT INFO
 * Context Field:
 * N_REPEAT, 	[15:8] Times to repeat
 * 			 	[7:0]  Delta
 * DELTA, 		[15:0] Delta
 * */
typedef struct {
	void (*callback) (void);
	EventLabel label;
	EventFlag flag;
	uint32_t schedule_time;
	uint16_t context;
} Event;

uint16_t eventContextFormat();
EventReturnCode eventRegister(void *callback, EventLabel label, EventFlag flag, uint16_t delta, uint8_t n_runs);
void eventClear();
EventReturnCode eventControllerInit(void);
EventReturnCode eventSchedule(uint8_t idx);

void eventRunner(void);
void eventDefaultCallback(void);
void eventTimerCallback(void);

#ifdef DEBUG_EVENT_CONTROLLER
void eventPrint(Event *event);
const char* EventReturnCodeToStr(EventReturnCode code);
const char* EventLabelToStr(EventLabel label);
const char* EventFlagToStr(EventFlag flag);
#endif

#endif /*EVENT_CONTROLLER*/
