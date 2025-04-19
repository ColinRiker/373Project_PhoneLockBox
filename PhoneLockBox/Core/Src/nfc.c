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

uint8_t poll_count;

void nfcInit(void) {
	uint8_t buff[255];

	PN532_I2C_Init(&pn532);
	PN532_GetFirmwareVersion(&pn532, buff);

#ifdef DEBUG_NFC
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
		//printf(".");
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

void nfcEventCallbackSlow(void) {
	if (nfcHasTarget()) {
		stateInsertFlag(SFLAG_NFC_PHONE_PRESENT);
	} else {
		stateInsertFlag(SFLAG_NFC_PHONE_NOT_PRESENT);
	}
}

void nfcEventCallbackStart(void) {
	uint8_t buff[PN532_FRAME_MAX_LENGTH];
	poll_count = 0;

	buff[0] = PN532_HOSTTOPN532;
	buff[1] = PN532_COMMAND_INLISTPASSIVETARGET & 0xFF;
	buff[2] = 0x01; // parm 1
	buff[3] = PN532_MIFARE_ISO14443A; // parm 2

	if (PN532_WriteFrame(&pn532, buff, 4) != PN532_ERROR_NONE) {
		eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 25, 0);

	} else {
		eventRegister(nfcEventCallbackPoll, EVENT_NFC_POLL, EVENT_SINGLE, 10, 0);

	}

}

void nfcEventCallbackPoll(void) {
	uint8_t status[] = {0x00};
	++poll_count;

	if (PN532_I2C_ReadData(status, sizeof(status)) != PN532_STATUS_OK) {
#ifdef DEBUG_NFC
		printf("[ERROR] NFC I2C poll failed\n\r");
#endif
	}

	if (status[0] == PN532_I2C_READY) {
		eventRegister(nfcEventCallbackRead, EVENT_NFC_READ, EVENT_SINGLE, 25, 0);
	} else {
		eventRegister(nfcEventCallbackPoll, EVENT_NFC_POLL, EVENT_SINGLE, 10, 0);
	}
}

void nfcEventCallbackRead(void) {
	uint8_t buff[MIFARE_UID_MAX_LENGTH] = {0};

	uint32_t frame_length = PN532_ReadFrame(&pn532, buff, 4);

	if (! ((buff[0] == PN532_PN532TOHOST) &&
			(buff[1] == (PN532_COMMAND_INLISTPASSIVETARGET + 1)))){
		//ERROR STATE
		//Restart?
	}

	if (frame_length - 2 >= 0) {
		//removeFlag();
		stateInsertFlag(SFLAG_NFC_PHONE_PRESENT);
	} else {
		//removeFlag(SFLAG_NFC_PHONE_PRESENT);
		stateInsertFlag(SFLAG_NFC_PHONE_NOT_PRESENT);
	}

	//We keep scheduling the start even incase the phone becomes present or is no longer present
	eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 25, 0);
}





