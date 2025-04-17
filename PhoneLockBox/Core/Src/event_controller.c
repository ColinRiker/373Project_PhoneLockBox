#include "event_controller.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdio.h>

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
		if (delta > 255) {
			delta = 255;
#ifdef DEBUG_EVENT_CONTROLLER
		printf("[ERROR] Delta too large for repeat event, setting to 255\n\r");
#endif
		}
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

void eventRemove(uint8_t idx) {
	events[idx].callback = eventDefaultCallback;
	events[idx].label = EVENT_EMPTY;
	events[idx].flag = EVENT_DISABLED;
	events[idx].schedule_time = MAX_TIME;
	events[idx].context = 0;
}

void eventClear(void) {
	for (uint8_t i = 0; i < MAX_EVENT_COUNT; ++i) {
		eventRemove(i);
	}
}

EventReturnCode eventControllerInit(void) {
	time_ms = 0;
	eventClear();

	if (HAL_TIM_Base_Start(&htim3) != HAL_OK) {
		return EVENT_INIT_FAILED;
	}

	return EVENT_SUCCESS;
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

#ifdef DEBUG_EVENT_CONTROLLER
	printf("Newly Scheduled ");
	eventPrint(&events[idx]);
#endif

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

#ifdef DEBUG_EVENT_CONTROLLER
void eventPrint(Event *event) {
	printf("Event Info, Label: %s, Flag: %s, Context: %x, Ptr: %x\n\r",
			EventLabelToStr(event->label), EventFlagToStr(event->flag), event->context, event);
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
