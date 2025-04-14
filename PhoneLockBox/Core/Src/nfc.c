/*
 * nfc.c
 *
 *  Created on: Apr 1, 2025
 *      Author: colinriker
 *
 */
#include "nfc.h"

#include <stdbool.h>
#include <stdio.h>

#include "pn532.h"
#include "event_controller.h"
#include "state_machine.h"


extern PN532 pn532;

void nfcInit(void) {
	uint8_t buff[255];

	PN532_I2C_Init(&pn532);
	PN532_GetFirmwareVersion(&pn532, buff);

#ifdef DEBUG_OUT
	if (PN532_GetFirmwareVersion(&pn532, buff) == PN532_STATUS_OK) {
		printf("Found PN532 with firmware version: %d.%d\r\n", buff[1], buff[2]);
	} else {
		printf("Error PN532 failed to get firmware version\r\n");
	}
#endif
	PN532_SamConfiguration(&pn532);
}


bool nfcHasTarget(void) {
	int32_t uid_len = 0;
	uint8_t uid[MIFARE_UID_MAX_LENGTH] = {0};

	uid_len = PN532_ReadPassiveTarget(&pn532, uid, PN532_MIFARE_ISO14443A, 1000);
	if (uid_len == PN532_STATUS_ERROR) {
		printf(".");
		return false;
	} else {
		printf("Found card with UID: ");
		for (uint8_t i = 0; i < uid_len; i++) {
			printf("%02x ", uid[i]);
		}
		printf("\r\n");
		return true;
	}

}

void nfcEventCallbackStart(void) {
	uint8_t buff[PN532_FRAME_MAX_LENGTH];

	buff[0] = PN532_HOSTTOPN532;
	buff[1] = PN532_COMMAND_INLISTPASSIVETARGET & 0xFF;
	buff[2] = 0x01; // parm 1
	buff[3] = PN532_MIFARE_ISO14443A; // parm 2

	PN532_WriteFrame(&pn532, buff, 4);

	eventRegister(nfcEventCallbackPoll, EVENT_NFC_POLL, EVENT_SINGLE, 1, 0);
}

void nfcEventCallbackPoll(void) {
	uint8_t status[] = {0x00};

	PN532_I2C_ReadData(status, sizeof(status));

	if (status[0] == PN532_I2C_READY) {
		eventRegister(nfcEventCallbackRead, EVENT_NFC_READ, EVENT_SINGLE, 1, 0);
	} else {
		eventRegister(nfcEventCallbackPoll, EVENT_NFC_POLL, EVENT_SINGLE, 1, 0);
	}
}

void nfcEventCallbackRead(void) {
	uint8_t buff[MIFARE_UID_MAX_LENGTH] = {0};

	PN532_ReadFrame(&pn532, buff, 4);
}




