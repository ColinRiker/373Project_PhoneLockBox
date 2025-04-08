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

void nfcInit(void);
bool nfcHasTarget(void);

#endif /* INC_NFC_H_ */
