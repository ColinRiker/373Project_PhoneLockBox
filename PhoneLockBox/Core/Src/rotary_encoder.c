/*
 * rotary_encoder.c
 *
 *  Created on: Mar 19, 2025
 *      Author: colinriker
 */

#include "rotary_encoder.h"

#include <stdint.h>

#include "shared.h"
#include "stm32l4xx_hal.h"


void rotencInit(void) {
	HAL_TIM_Encoder_Start_IT(&htim1, TIM_CHANNEL_ALL);
}



BoxMode rotencResolve(void) {
	switch (state.mode) {
		case UNLOCKED_EMPTY_ASLEEP:

			break;
		case UNLOCKED_ASLEEP_TO_AWAKE:

			break;
		case UNLOCKED_EMPTY_AWAKE:

			break;
		case UNLOCKED_FULL_AWAKE_FUNC_A:

			break;
		case UNLOCKED_FULL_AWAKE_FUNC_B:

			break;
		case UNLOCKED_FULL_ASLEEP:

			break;
		case UNLOCKED_TO_LOCKED_AWAKE:

			break;
		case LOCKED_FULL_AWAKE:

			break;
		case LOCKED_FULL_ASLEEP:

			break;
		case LOCKED_MONITOR_AWAKE:

			break;
		case LOCKED_MONITOR_ASLEEP:

			break;
		case LOCKED_FULL_NOTIFICATION_FUNC_A:

			break;
		case LOCKED_FULL_NOTIFICATION_FUNC_B:

			break;
		case EMERGENCY_OPEN:

			break;
		default:

		break;
		}
}
