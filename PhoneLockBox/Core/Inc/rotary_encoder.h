/*
 * rotary_encoder.h
 *
 *  Created on: Mar 19, 2025
 *      Author: colinriker
 */

#ifndef INC_ROTARY_ENCODER_H_
#define INC_ROTARY_ENCODER_H_

#include "shared.h"

#include <stdint.h>
#include "stm32l4xx_hal.h"

extern BoxState state;
extern BoxState next_state;
extern TIM_HandleTypeDef htim1;

void rotencInit(void);
uint16_t rotencRead(void);
void rotencResolve(void);

#endif /* INC_ROTARY_ENCODER_H_ */
