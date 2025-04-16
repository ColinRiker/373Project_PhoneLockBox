#include "event_controller.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "state_machine.h"

#include "stm32l4xx_hal.h"

extern TIM_HandleTypeDef htim3;

/* Event Data Structures */
Event events[MAX_EVENT_COUNT];
uint32_t time_ms;

/* Event Functions */
EventReturnCode eventRegister(void *callback, EventLabel label, EventFlag flag, uint16_t delta, uint8_t n_runs) {
	uint16_t context = delta;
	if (flag == EVENT_N_REPEAT || flag == EVENT_N_REPEAT_IMMEDIATE) {
		context = (n_runs << 8) | (context & 0x00FF);
	}

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


EventReturnCode eventClear(void) {
	for (uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		events[i].callback = eventDefaultCallback;
		events[i].label = EVENT_EMPTY;
		events[i].flag = EVENT_DISABLED;
		events[i].schedule_time = MAX_TIME;
		events[i].context = 0;
	}
	return EVENT_SUCCESS;
}


EventReturnCode eventControllerInit(void) {
	for (uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		events[i].callback = eventDefaultCallback;
		events[i].label = EVENT_EMPTY;
		events[i].flag = EVENT_DISABLED;
		events[i].schedule_time = MAX_TIME;
		events[i].context = 0;
	}

	if (HAL_TIM_Base_Start(&htim3) != HAL_OK) {
		return EVENT_INIT_FAILED;
	}

	return EVENT_SUCCESS;
}

EventReturnCode eventRemove(uint8_t idx) {
	events[idx].callback = eventDefaultCallback;
	events[idx].label = EVENT_EMPTY;
	events[idx].flag = EVENT_DISABLED;
	events[idx].schedule_time = MAX_TIME;
	events[idx].context = 0;
}

EventReturnCode eventSchedule(uint8_t idx) {
	uint8_t schedule_offset = time_ms % 7; //Hopefully helps to cheaply redistribute scheduling

	switch(events[idx].flag) {
	case EVENT_SINGLE:
		events[idx].schedule_time = time_ms + schedule_offset + events[idx].context;
	case EVENT_SINGLE_IMMEDIATE:
		events[idx].schedule_time = time_ms + events[idx].context;
	case EVENT_DELTA:
		events[idx].schedule_time = time_ms + schedule_offset;
	case EVENT_DELTA_IMMEDIATE:
		events[idx].schedule_time = time_ms;
	case EVENT_N_REPEAT:
		events[idx].schedule_time = time_ms + schedule_offset;
	case EVENT_N_REPEAT_IMMEDIATE:
		events[idx].schedule_time = time_ms;
	default:
		return EVENT_GENERIC_ERROR;
	}
	return EVENT_SUCCESS;
}

void eventRunner(void) {
	for (uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		if (events[i].schedule_time <= time_ms) {
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

void eventTimerCallback(void) {
	stateInsertFlag(SFLAG_TIMER_COMPLETE);
}

void eventDefaultCallback(void) {
#ifdef DEBUG_EVENT_CONTROLLER
	printf("[ERROR] Default Event Callback called\n\r");
#endif
}
