#include "event_controller.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "shared.h"
#include "state_machine.h"
#include "stm32l4xx_hal.h"

extern TIM_HandleTypeDef htim3;

/* Event Global Declarations
 *	events: stores all active event records currently scheduled
 *	time_ms: is the time reference incremented by the timer interrupt
 */
Event events[MAX_EVENT_COUNT];
uint32_t time_ms;

/* eventRegister()
 *  	Creates an event record from the pass paramters and
 *	calls eventSchedule on the new record after adding it to
 *	the events array. Acts as the entry point for interaction
 *	with the event system.
 */
EventReturnCode eventRegister(void *callback, EventLabel label, EventFlag flag, uint16_t delta, uint8_t n_runs) {
	uint16_t context = delta;

	if (flag == EVENT_N_REPEAT || flag == EVENT_N_REPEAT_IMMEDIATE) {
		if (delta > 255) {
			delta = 255;
#ifdef DEBUG_EVENT_CONTROLLER
			printf("[ERROR] Delta too large for repeat event, setting to 255\n\r");
#endif
		}
		context = (n_runs << 8) | (context & 0x00FF);
	}
#ifdef DEBUG_EVENT_CONTROLLER
	printf("[INFO] event registration called with, %s, %s, %u %u\n\r", EventLabelToStr(label), EventFlagToStr(flag), delta, n_runs);
#endif 
	for (uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (events[i].label == EVENT_EMPTY) {
			events[i].callback = callback;
			events[i].label = label;
			events[i].flag = flag;
			events[i].context = context;
			eventSchedule(i);
			return EVENT_SUCCESS;
		}
	}

	return EVENT_QUEUE_FULL;
}

void eventRemove(uint8_t idx) {
#ifdef DEBUG_EVENT_CONTROLLER
	printf("[INFO] Removing Event");
	eventPrint(&events[idx]);
#endif

	events[idx].callback = eventDefaultCallback;
	events[idx].label = EVENT_EMPTY;
	events[idx].flag = EVENT_DISABLED;
	events[idx].schedule_time = MAX_TIME;
	events[idx].context = 0;
}

/* eventClear()
 *	walks the events array removing any events found
 */
void eventClear(void) {
	for (uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		if(events[i].label != EVENT_EMPTY) {
			eventRemove(i);
		}
	}
}

EventReturnCode eventControllerInit(void) {
	time_ms = 0;
	eventClear();

	if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK) {
		printf("[ERROR] Timer 3 did not start\n\r");
		return EVENT_INIT_FAILED;
	}

	printf("[INFO] Event system initalization complete\n\r");
	return EVENT_SUCCESS;
}

/* eventSchedule()
 *	takes the idx of an event and depending on its scheduling flag
 *	sets it's scheduling time for when the eventRunner should call
 *	its callback. Immedaiates, N repeats
 */
EventReturnCode eventSchedule(uint8_t idx) {
	uint8_t schedule_offset = time_ms % 7; //Hopefully helps to cheaply redistribute scheduling

	switch(events[idx].flag) {
	case EVENT_SINGLE:
		events[idx].schedule_time = time_ms + schedule_offset + events[idx].context;
		break;
	case EVENT_SINGLE_IMMEDIATE:
		events[idx].schedule_time = time_ms + events[idx].context;
		break;
	case EVENT_DELTA:
		events[idx].schedule_time = time_ms + schedule_offset;
		break;
	case EVENT_DELTA_IMMEDIATE:
		events[idx].schedule_time = time_ms;
		break;
	case EVENT_N_REPEAT:
		events[idx].schedule_time = time_ms + schedule_offset;
		break;
	case EVENT_N_REPEAT_IMMEDIATE:
		events[idx].schedule_time = time_ms;
		break;
	default:
#ifdef DEBUG_EVENT_CONTROLLER
		printf("[ERROR] Bad event type, not scheduled, ID: %ls\n\r", (int*) &events[idx]);
#endif 
		return EVENT_GENERIC_ERROR;
	}

#ifdef DEBUG_EVENT_CONTROLLER
	printf("[INFO] Scheduled ");
	eventPrint(&events[idx]);
#endif

	return EVENT_SUCCESS;
}

/* eventRunner()
 * 	This function serves as the executor of event callbacks. It simply
 * 	iterates the event loop looking for valid and ready to be run events.
 * 	After it calls an events callback it executes the switch statement to
 * 	see how rescheduling should be handled. Immedates are downgraded to
 * 	their non-immeidate varities. N repeat events have their context fields
 * 	motifided to decrement how many more runs they have. Once they get to
 * 	a single remaining run they become singles.
 * */
void eventRunner(void) {
	for (uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (events[i].schedule_time <= time_ms && events[i].label != EVENT_EMPTY) {

			events[i].callback();

			//Reschedule or Remove Handler
			switch(events[i].flag) {
			case EVENT_DELTA:
				events[i].schedule_time = time_ms + events[i].context;
				break;

			case EVENT_DELTA_IMMEDIATE:
				events[i].flag = EVENT_DELTA;
				events[i].schedule_time = time_ms + events[i].context;
				break;

			case EVENT_N_REPEAT_IMMEDIATE: {
				events[i].flag = EVENT_N_REPEAT;

				uint8_t n = (events[i].context & 0xFF00) >> 8;
				if (n > 1) {
					events[i].context = ((n - 1) << 8) | (events[i].context & 0x00FF);
				} else {
					events[i].flag = EVENT_SINGLE;
					events[i].context &= 0x00FF;
				}

				events[i].schedule_time = time_ms + events[i].context;
				break;
			}

			case EVENT_N_REPEAT: {
				uint8_t n = (events[i].context & 0xFF00) >> 8;
				if (n > 1) {
					events[i].context = ((n - 1) << 8) | (events[i].context & 0x00FF);
				} else {
					events[i].flag = EVENT_SINGLE;
					events[i].context &= 0x00FF;
				}

				events[i].schedule_time = time_ms + events[i].context;
				break;
			}

			default:
				eventRemove(i);
			}
		}
	}

}

/* eventTimerCallback()
 *	A simple callback to insert the timer complete flag after a
 *	scheduled amount of time, used for timeouts typically
 */
void eventTimerCallback(void) {
	stateInsertFlag(SFLAG_TIMER_COMPLETE);
}

/* eventDefaultCallback()
 *	on remove or intialization all events are assigned this
 *	callback to ensure we're never calling into random areas
 *	and provideds debugging if we have misbehaving or bad events
 */
void eventDefaultCallback(void) {
#ifdef DEBUG_EVENT_CONTROLLER
	printf("[ERROR] Default Event Callback called\n\r");
#endif
}

/* Debugging Functions
 * 	Below are all the debugging functions needed for the event system
 * 	and its structs/enums
 */
#ifdef DEBUG_EVENT_CONTROLLER
void eventPrint(Event *event) {
	printf("Label: %s, Flag: %s, Context: %x, Ptr: %lu\n\r",
			EventLabelToStr(event->label), EventFlagToStr(event->flag), event->context, (uint32_t)event);
}

const char* EventReturnCodeToStr(EventReturnCode code) {
	switch (code) {
	case EVENT_SUCCESS: return "EVENT_SUCCESS";
	case EVENT_LABEL_NOT_FOUND: return "EVENT_LABEL_NOT_FOUND";
	case EVENT_LABEL_ALREADY_USED: return "EVENT_LABEL_ALREADY_USED";
	case EVENT_INIT_FAILED: return "EVENT_INIT_FAILED";
	case EVENT_QUEUE_FULL: return "EVENT_QUEUE_FULL";
	case EVENT_GENERIC_ERROR: return "EVENT_GENERIC_ERROR";
	default: return "UNKNOWN_RC";
	}
}

const char* EventLabelToStr(EventLabel label) {
	switch (label) {
	case EVENT_EMPTY: return "EVENT_EMPTY";
	case EVENT_NFC_START_READ: return "EVENT_NFC_START_READ";
	case EVENT_NFC_POLL: return "EVENT_NFC_POLL";
	case EVENT_NFC_READ: return "EVENT_NFC_READ";
	case EVENT_ROTARY_ENCODER: return "EVENT_ROTARY_ENCODER";
	case EVENT_AUDIO: return "EVENT_AUDIO";
	case EVENT_TIMER: return "EVENT_TIMER";
	case EVENT_ACCELEROMETER: return "EVENT_ACCELEROMETER";
	default: return "UNKNOWN_LABEL";
	}
}

const char* EventFlagToStr(EventFlag flag) {
	switch (flag) {
	case EVENT_DISABLED: return "EVENT_DISABLED";
	case EVENT_SINGLE: return "EVENT_SINGLE";
	case EVENT_SINGLE_IMMEDIATE: return "EVENT_SINGLE_IMMEDIATE";
	case EVENT_DELTA: return "EVENT_DELTA";
	case EVENT_DELTA_IMMEDIATE: return "EVENT_DELTA_IMMEDIATE";
	case EVENT_N_REPEAT: return "EVENT_N_REPEAT";
	case EVENT_N_REPEAT_IMMEDIATE: return "EVENT_N_REPEAT_IMMEDIATE";
	default: return "UNKNOWN_FLAG";
	}
}

#endif
