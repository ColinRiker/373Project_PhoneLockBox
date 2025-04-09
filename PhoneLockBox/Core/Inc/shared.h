/*
 * shared.h
 *
 *  Created on: Mar 28, 2025
 *      Author: colinriker
 */

#ifndef INC_SHARED_H_
#define INC_SHARED_H_

#define DEBUG_OUT //Comment out to not compile debug functions and statement

#ifdef DEBUG_OUT
#define DEBUG_BUFFER_SIZE 100
#define DEBUG_EVENT_CONTROLLER
#define DEBUG_NFC
#define DEBUG_DISPLAY
#define DEBUG_AUDIO
#endif /*END DEBUG DEFINES*/

#include <string.h>
#include <stdint.h>

enum {
	UNLOCKED_EMPTY_ASLEEP,
	UNLOCKED_ASLEEP_TO_AWAKE,
	UNLOCKED_EMPTY_AWAKE,
	UNLOCKED_FULL_AWAKE_FUNC_A,
	UNLOCKED_FULL_AWAKE_FUNC_B,
	UNLOCKED_FULL_ASLEEP,
	UNLOCKED_TO_LOCKED_AWAKE,
	LOCKED_FULL_AWAKE,
	LOCKED_FULL_ASLEEP,
	LOCKED_MONITOR_AWAKE,
	LOCKED_MONITOR_ASLEEP,
	LOCKED_FULL_NOTIFICATION_FUNC_A,
	LOCKED_FULL_NOTIFICATION_FUNC_B,
	EMERGENCY_OPEN
} typedef BoxMode;

enum {
	NO_INTERRUPT,
	ENCODER_INTERRUPT,
	MICROPHONE_INTERRUPT,
	ENCODER_AND_MICROPHONE_INTERRUPT
} typedef BoxInterruptFlag;

struct {
	BoxMode mode;
	BoxInterruptFlag interrupt_flag;
} typedef BoxState;

struct {
	int16_t x_componenet;
	int16_t y_componenet;
	int16_t z_componenet;
} typedef Vector3D;

/* Global Variables */
extern BoxState state;
extern BoxState next_state;
/* End Global Variables */

static inline uint16_t VectorDelta(Vector3D *vec, Vector3D *prev_vec){
	return (vec->x_componenet - prev_vec->x_componenet) +
			(vec->y_componenet - prev_vec->y_componenet) +
			(vec->z_componenet - prev_vec->z_componenet);
}

static inline void interruptFlagToStr(char* buffer, BoxInterruptFlag flag) {
	switch (flag) {
	case NO_INTERRUPT:
		strncpy(buffer, "No Interrupt", DEBUG_BUFFER_SIZE);
		break;
	case ENCODER_INTERRUPT:
		strncpy(buffer, "Encoder Interrupt", DEBUG_BUFFER_SIZE);
		break;
	case MICROPHONE_INTERRUPT:
		strncpy(buffer, "Microphone Interrupt", DEBUG_BUFFER_SIZE);
		break;
	case ENCODER_AND_MICROPHONE_INTERRUPT:
		strncpy(buffer, "Encoder & Microphone Interrupt", DEBUG_BUFFER_SIZE);
		break;
	default:
		strncpy(buffer, "Error", DEBUG_BUFFER_SIZE);
		break;
	}
}

static inline void stateToStr(char* buffer, BoxMode mode) {

	switch (mode) {
	case UNLOCKED_EMPTY_ASLEEP:
		strncpy(buffer, "Unlocked Empty Asleep", DEBUG_BUFFER_SIZE);
		break;
	case UNLOCKED_ASLEEP_TO_AWAKE:
		strncpy(buffer, "Unlocked Asleep to Awake", DEBUG_BUFFER_SIZE);
		break;
	case UNLOCKED_EMPTY_AWAKE:
		strncpy(buffer, "Unlocked Empty Awake", DEBUG_BUFFER_SIZE);
		break;
	case UNLOCKED_FULL_AWAKE_FUNC_A:
		strncpy(buffer, "Unlocked Full Awake Function A", DEBUG_BUFFER_SIZE);
		break;
	case UNLOCKED_FULL_AWAKE_FUNC_B:
		strncpy(buffer, "Unlocked Full Awake Function B", DEBUG_BUFFER_SIZE);
		break;
	case UNLOCKED_FULL_ASLEEP:
		strncpy(buffer, "Unlocked Full Asleep", DEBUG_BUFFER_SIZE);
		break;
	case UNLOCKED_TO_LOCKED_AWAKE:
		strncpy(buffer, "Unlocked to Locked Awake", DEBUG_BUFFER_SIZE);
		break;
	case LOCKED_FULL_AWAKE:
		strncpy(buffer, "Locked Full Awake", DEBUG_BUFFER_SIZE);
		break;
	case LOCKED_FULL_ASLEEP:
		strncpy(buffer, "Locked Full Asleep", DEBUG_BUFFER_SIZE);
		break;
	case LOCKED_MONITOR_AWAKE:
		strncpy(buffer, "Locked Monitor Awake", DEBUG_BUFFER_SIZE);
		break;
	case LOCKED_MONITOR_ASLEEP:
		strncpy(buffer, "Locked Monitor Asleep", DEBUG_BUFFER_SIZE);
		break;
	case LOCKED_FULL_NOTIFICATION_FUNC_A:
		strncpy(buffer, "Locked Full Notification Function A", DEBUG_BUFFER_SIZE);
		break;
	case LOCKED_FULL_NOTIFICATION_FUNC_B:
		strncpy(buffer, "Locked Full Notification Function B", DEBUG_BUFFER_SIZE);
		break;
	case EMERGENCY_OPEN:
		strncpy(buffer, "Emergency Open", DEBUG_BUFFER_SIZE);
		break;
	default:
		strncpy(buffer, "Error", DEBUG_BUFFER_SIZE);
		break;
	}
}

#endif /* INC_SHARED_H_ */
