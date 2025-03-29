/*
 * display_logic.h
 *
 *  Created on: Mar 28, 2025
 *      Author: bellalongo
 */

#ifndef SRC_DISPLAY_LOGIC_H_
#define SRC_DISPLAY_LOGIC_H_

#include <stdint.h>

void Display_TurnOff(void);
void Display_ShowStartupPrompt(void);
void Display_ShowSetTime(uint8_t minutes, uint8_t selected);
void Display_ShowForceOpen(void);

#endif /* SRC_DISPLAY_LOGIC_H_ */

