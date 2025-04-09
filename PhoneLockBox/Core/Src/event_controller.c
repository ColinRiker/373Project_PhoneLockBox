#include "event_controller.h"

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

/* Event Data Structures */
Event events[MAX_EVENT_COUNT];

/* Event Functions */
EventReturnCode eventRegister(void *callback, EventLabel label, EventFlag flag, uint16_t period) {

	uint8_t empty_idx = MAX_EVENT_COUNT;

	for (uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (events[i].label == label) {
			return EVENT_LABEL_ALREADY_USED;
		}

		if (events[i].label == EVENT_EMPTY && empty_idx == MAX_EVENT_COUNT) {
			empty_idx = i;
		}
	}
	return EVENT_SUCCESS;
}

EventReturnCode eventRemove(EventLabel label) {

	//Checks to see if an not queued event entry exists
	for (uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (events[i].label == label) {
			events[i].callback = eventDefaultCallback;
			events[i].label = EVENT_EMPTY;
			events[i].flag = EVENT_DISABLED;
			events[i].period = 0;

#ifdef DEBUG_EVENT_CONTROLLER
			//char label[10] = {0};
			printf("[INFO] Event dequeued and deleted from storage array");
#endif /*END DEBUG_EVENT_CONTROLLER*/

			return EVENT_SUCCESS;
		}
	}
	return EVENT_LABEL_NOT_FOUND;
}

EventReturnCode eventUpdate(EventLabel label, EventFlag new_flag, uint16_t new_period) {
	for(uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (events[i].label == label) {
			events[i].flag = new_flag;
			events[i].period = new_period;
			return EVENT_SUCCESS;
		}
	}
	return EVENT_LABEL_NOT_FOUND;
}

EventReturnCode eventControllerInit(void) {
	for (uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		events[i].callback = eventDefaultCallback;
		events[i].label = EVENT_EMPTY;
		events[i].flag = EVENT_DISABLED;
		events[i].period = 0;
	}

}

void eventScheduler(void) {
	//On Register we append new events to the queue
	//Time is the execution time, eg when the timer equals 5, run event b (which should be our head)
	//Delta is for when the next copy of the event should be run

	//
	//
	//
}

void eventDefaultCallback(void) {
#ifdef DEBUG_EVENT_CONTROLLER
	printf("[ERROR] Default Event Callback called\n\r");
#endif
}
