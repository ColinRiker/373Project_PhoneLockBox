/*
 * audio.h
 *
 *  Created on: Apr 9, 2025
 *      Author: colinrik
 */

#ifndef INC_AUDIO_H_
#define INC_AUDIO_H_

#define MAX_ENTRIES 100
#define TIME_MIN_THRESH 0.0025
#define TIME_MAX_THRESH 0.0030

#include <stdbool.h>

void audioInit(void);
void audioCount(void);
void audioEventCallback(void);
bool audioMatch(void);

#endif /* INC_AUDIO_H_ */
