/*
 * rotary_encoder.h
 *
 *  Created on: Mar 19, 2025
 *      Author: colinriker
 */

#ifndef INC_ROTARY_ENCODER_H_
#define INC_ROTARY_ENCODER_H_


#include <stdint.h>
#include <stdbool.h>

#include "shared.h"
#include "stm32l4xx_hal.h"

void rotencInit(void);
uint32_t rotencGetDelta(void);
bool rotencHasMoved(void);
void rotencDeltaEvent(void);

#endif /* INC_ROTARY_ENCODER_H_ */
