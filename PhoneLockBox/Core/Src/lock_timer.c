/*
 * lock.c
 *
 *  Created on: Apr 15, 2025
 *      Author: colinriker
 */
#include "lock_timer.h"

#include <stdbool.h>
#include <stdint.h>

#include "stm32l4xx_hal.h"

extern bool master_timer_done;  // indicates whether the master timer has finished
extern TIM_HandleTypeDef htim2;  // timer handle for the timer used for the lock

uint32_t max_time_ms;  // stores the maximum time for the lock timer (in milliseconds)

// Initializes the lock timer and prepares it to start with a set time
void lockTimerInit(void) {
    lockDisenage();         // ensure the lock is disengaged initially
    lockTimerCancel();      // cancel any ongoing timer
    lockTimerSetTime(10000); // set the timer to 10 seconds (10000 ms)
    master_timer_done = false;  // reset the flag indicating the timer is not done
}

// Starts the lock timer by getting the current time, enabling the timer interrupt, and resetting the done flag
void lockTimerStart(void) {
    max_time_ms = lockTimerGetTime();  // get the current time in milliseconds
    HAL_TIM_Base_Start_IT(&htim2);     // start the timer interrupt (trigger interrupt when time is reached)
    master_timer_done = false;         // ensure the timer done flag is reset
}

// Returns the current timer value in milliseconds by reading the timer counter (divided by 10 for time unit conversion)
uint32_t lockTimerGetTime(void) {
    return TIM2->CNT / 10;  // return the current timer counter value divided by 10 (to convert from microseconds to milliseconds)
}

// Sets the lock timer to a specific time (in milliseconds)
void lockTimerSetTime(int32_t time) {
    if (time < 0)  // if the time is negative, set it to zero
        time = 0;

    TIM2->CNT = time * 10;  // set the timer counter value based on the time (converted to microseconds)
}

// Cancels the lock timer by stopping the timer interrupt
void lockTimerCancel(void) {
    HAL_TIM_Base_Stop_IT(&htim2);  // stop the timer interrupt (no further interrupts will be triggered)
}

// Engages the lock by setting the GPIO pin high (locking the system)
void lockEngage(void) {
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, 1);  // set the GPIO pin PE15 high to engage the lock
}

// Disengages the lock by setting the GPIO pin low (unlocking the system)
void lockDisenage(void) {
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, 0);  // set the GPIO pin PE15 low to disengage the lock
}
