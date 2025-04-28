/*
 * audio.c
 *
 *  Created on: Apr 9, 2025
 *      Author: colinrik
 */

#include "audio.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "shared.h"
#include "state_machine.h"
#include "event_controller.h"
#include "stm32l4xx_hal.h"

extern TIM_HandleTypeDef htim2;  // external timer handle for time management
extern uint32_t time_ms;         // external time counter (in milliseconds)
extern SFlag flags[MAX_FLAGS];   // array to store state flags

uint32_t matrix[MAX_ENTRIES] = {0};  // stores the time deltas between audio events (ring buffer)
uint8_t audio_count;                 // counter for the number of audio interrupts detected
uint32_t last_interrupt_time;        // timestamp of the last interrupt

// Initializes the audio system, starting the timer and setting initial values for counters
void audioInit(void) {
    HAL_TIM_Base_Start(&htim2);  // start the timer used for measuring time intervals
    audio_count = 0;             // reset the audio count
    last_interrupt_time = 0;     // reset the timestamp for the last interrupt
}

// Compares the time deltas between audio events to determine if there's an audio match
bool audioMatch(void) {
    uint32_t sum_deltas = 0;  // total sum of time differences between events
    float time_total = 0.0;   // total time calculated from the deltas

#ifdef DEBUG_AUDIO
    printf("=== Last %d Interrupts (ring buffer) ===\n", MAX_ENTRIES);
    // Debugging: print out the time deltas of the last MAX_ENTRIES audio events
    for (int i = 0; i < MAX_ENTRIES; i++) {
        printf("Event %d: Δt = %lu us\n\r", i, matrix[i]);
    }
#endif /* END DEBUG_AUDIO */

    // Calculate the total time from all the time deltas in the matrix
    for (int r = 0; r < MAX_ENTRIES; r++) {
        sum_deltas += matrix[r];  // accumulate all time deltas
        time_total = (float)sum_deltas / 1000000.0f;  // convert to seconds
    }

#ifdef DEBUG_AUDIO
    printf("time gap total = %f seconds\n", time_total);  // Debugging: print the total time in seconds
#endif /* END DEBUG_AUDIO */

    audio_count = 0;  // reset audio count after processing
    // Clear the matrix to start fresh for the next batch of events
    for (uint8_t i = 0; i < MAX_ENTRIES; ++i) {
        matrix[i] = 0;  // reset each entry in the matrix
    }

    // Return true if the total time between events is within the acceptable threshold range
    return (time_total <= TIME_MAX_THRESH) && (time_total >= TIME_MIN_THRESH);
}

// Callback function to handle an audio interrupt and store the time delta between events
void audioEventCallback(void) {
    uint32_t now = time_ms;        // get the current time
    uint32_t delta;                // variable to store the time difference between this and the last interrupt

    // Calculate the time difference (delta) between the current and the last interrupt time
    if (now >= last_interrupt_time) {
        delta = now - last_interrupt_time;  // normal case: current time is later than the last interrupt
    } else {
        delta = (2000000 - last_interrupt_time) + now;  // account for time overflow if needed
    }

    last_interrupt_time = now;  // update the timestamp for the last interrupt
    ++audio_count;              // increment the audio interrupt count

    // If we haven't yet reached the maximum number of events, store the delta
    if (audio_count < MAX_ENTRIES) {
        matrix[audio_count] = delta;  // store the delta in the matrix
    }

#ifdef DEBUG_AUDIO
    // Debugging: print the interrupt count and the current time of the interrupt
    printf("Interrupt #%d triggered at %ld µs \n", audio_count, now);
#endif /* END DEBUG_AUDIO */

    // Once we reach the maximum number of entries, check if there's an audio match
    if (audio_count == MAX_ENTRIES) {
        if (audioMatch()) {  // if the time total is within the acceptable range, we have a match
            stateRemoveFlag(SFLAG_AUDIO_NO_MATCH);  // remove the flag indicating no match
            stateInsertFlag(SFLAG_AUDIO_MATCH);     // insert the flag indicating a match
        } else {  // if the time total is outside the threshold, there's no match
            stateRemoveFlag(SFLAG_AUDIO_MATCH);  // remove the flag indicating a match
            stateInsertFlag(SFLAG_AUDIO_NO_MATCH);  // insert the flag indicating no match
        }
    }
}

