/*
 * shared.h
 *
 *  Created on: Mar 28, 2025
 *      Author: colinriker
 */

#ifndef INC_SHARED_H_
#define INC_SHARED_H_


#define I2C_TIMEOUT 1000
#define DEBUG_OUT //Comment out to not compile debug functions and statement

#ifdef DEBUG_OUT
#define DEBUG_EVENT_CONTROLLER
#define DEBUG_NFC
//#define DEBUG_DISPLAY
//#define DEBUG_AUDIO
#define DEBUG_ROTARY_ENCODER
#define DEBUG_STATE_CONTROLLER
//#define DEBUG_ACC_MAG
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
} typedef BoxState;


typedef struct {
	BoxState mode;
} BoxState_old;

typedef struct {
	int16_t x_componenet;
	int16_t y_componenet;
	int16_t z_componenet;
} Vector3D;

/* Global Variables */
extern BoxState state;
extern BoxState next_state;
/* End Global Variables */

static inline uint16_t VectorDelta(Vector3D *vec, Vector3D *prev_vec){
	return (vec->x_componenet - prev_vec->x_componenet) +
			(vec->y_componenet - prev_vec->y_componenet) +
			(vec->z_componenet - prev_vec->z_componenet);
}


static inline const char* stateToStr(BoxState boxstate) {

	switch (boxstate) {
	case UNLOCKED_EMPTY_ASLEEP: return "Unlocked Empty Asleep";

	case UNLOCKED_ASLEEP_TO_AWAKE: return "Unlocked Asleep to Awake";

	case UNLOCKED_EMPTY_AWAKE: return "Unlocked Empty Awake";

	case UNLOCKED_FULL_AWAKE_FUNC_A: return "Unlocked Full Awake Function A";

	case UNLOCKED_FULL_AWAKE_FUNC_B: return "Unlocked Full Awake Function B";

	case UNLOCKED_FULL_ASLEEP: return "Unlocked Full Asleep";

	case UNLOCKED_TO_LOCKED_AWAKE: return "Unlocked to Locked Awake";

	case LOCKED_FULL_AWAKE: return "Locked Full Awake";

	case LOCKED_FULL_ASLEEP: return "Locked Full Asleep";

	case LOCKED_MONITOR_AWAKE: return "Locked Monitor Awake";

	case LOCKED_MONITOR_ASLEEP: return "Locked Monitor Asleep";

	case LOCKED_FULL_NOTIFICATION_FUNC_A: return "Locked Full Notification Function A";

	case LOCKED_FULL_NOTIFICATION_FUNC_B: return "Locked Full Notification Function B";

	case EMERGENCY_OPEN: return "Emergency Open";

	default: return "[ERROR] Undefined State";
	}
}

#endif /* INC_SHARED_H_ */
