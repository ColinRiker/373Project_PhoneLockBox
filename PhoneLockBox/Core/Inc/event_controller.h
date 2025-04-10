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

#define MAX_EVENT_COUNT 10 /* MUST BE LESS THAN 256 */
#define MAX_TIME 0xFFFF

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
	EVENT_NFC_READ,
	EVENT_ROTARY_ENCODER,
	EVENT_AUDIO,
	EVENT_TIMER
} EventLabel;

typedef enum {
	EVENT_DISABLED,
	EVENT_SINGLE,
	EVENT_SIGNLE_IMMEDIATE,
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
	uint16_t schedule_time;
	uint16_t context;
} Event;

uint16_t eventContextFormat()
EventReturnCode eventRegister(void *callback, EventLabel label, EventFlag flag, uint16_t period);
EventReturnCode eventClear();
EventReturnCode eventControllerInit(void);
EventReturnCode eventSchedule(uint8_t idx);
EventReturnCode eventRemove(uint8_t idx);
void eventRunner(void);
void eventDefaultCallback(void);

//Debug Functions
void eventLabelToStr(char* buffer);
void eventFlagToStr(char* buffer);

#endif /*EVENT_CONTROLLER*/
