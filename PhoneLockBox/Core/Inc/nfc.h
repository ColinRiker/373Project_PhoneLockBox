/*
 * nfc.h
 *
 *  Created on: Apr 1, 2025
 *      Author: colinriker
 */

#ifndef INC_NFC_H_
#define INC_NFC_H_

#include <stdbool.h>
#include "pn532.h"

#define MAX_POLL 100

void nfcInit(void);
bool nfcHasTarget(void);

void nfcEventCallbackStart(void);
void nfcResetPinHigh(void);
void nfcInitSequence(void);
void nfcEventCallbackPoll(void);
void nfcEventCallbackRead(void);
bool testPN532Communication(void);

#endif /* INC_NFC_H_ */
