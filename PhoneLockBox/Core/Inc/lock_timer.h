/*
 * lock_timer.h
 *
 *  Created on: Apr 15, 2025
 *      Author: colinriker
 */

#ifndef INC_LOCK_TIMER_H_
#define INC_LOCK_TIMER_H_

#include <stdint.h>

void lockTimerInit(void);
void lockTimerStart(void);
uint32_t lockTimerGetTime(void);
void lockTimerSetTime(int32_t time);
void lockTimerCancel(void);

void lockEngage(void);
void lockDisenage(void);

#endif /* INC_LOCK_TIMER_H_ */
