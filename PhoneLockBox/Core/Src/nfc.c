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

extern PN532 pn532;  // external reference to the PN532 NFC module

uint8_t poll_count;  // counter for NFC polling events

// Initializes the NFC module, configures the PN532, and gets the firmware version
void nfcInit(void) {
    uint8_t buff[255];  // buffer for receiving firmware version

    PN532_I2C_Init(&pn532);  // initialize the NFC module via I2C
    PN532_GetFirmwareVersion(&pn532, buff);  // get the firmware version from the NFC module

#ifdef DEBUG_NFC
    // Debugging: print the firmware version of the PN532 if successful
    if (PN532_GetFirmwareVersion(&pn532, buff) == PN532_STATUS_OK) {
        printf("Found PN532 with firmware version: %d.%d\r\n", buff[1], buff[2]);
    } else {
        // Debugging: print an error message if the firmware version could not be retrieved
        printf("Error PN532 failed to get firmware version\r\n");
    }
#endif

    PN532_SamConfiguration(&pn532);  // configure the PN532 for passive mode
}

// Checks if there is an NFC target (phone) present by attempting to read its UID
bool nfcHasTarget(void) {
    int32_t uid_len = 0;
    uint8_t uid[MIFARE_UID_MAX_LENGTH] = {0};  // buffer to store the UID of the detected NFC target

    // Attempt to read the passive target (NFC phone)
    uid_len = PN532_ReadPassiveTarget(&pn532, uid, PN532_MIFARE_ISO14443A, 1000);

    // If an error occurs while reading the target, return false
    if (uid_len == PN532_STATUS_ERROR) {
        return false;
    } else {
        // Debugging: print the UID of the detected NFC phone
        printf("Found card with UID: ");
        for (uint8_t i = 0; i < uid_len; i++) {
            printf("%02x ", uid[i]);
        }
        printf("\r\n");
        return true;  // return true if a target was detected
    }
}

// Callback function for handling NFC events with slower polling intervals
void nfcEventCallbackSlow(void) {
    // If an NFC target is detected, update the flags to reflect the phone's presence
    if (nfcHasTarget()) {
        stateRemoveFlag(SFLAG_NFC_PHONE_NOT_PRESENT);  // remove the flag indicating phone is not present
        stateInsertFlag(SFLAG_NFC_PHONE_PRESENT);      // insert the flag indicating phone is present
    } else {
        stateInsertFlag(SFLAG_NFC_PHONE_NOT_PRESENT);  // insert the flag indicating phone is not present
        stateRemoveFlag(SFLAG_NFC_PHONE_PRESENT);      // remove the flag indicating phone is present
    }
}

// Callback function to initiate reading from the NFC module (start polling)
void nfcEventCallbackStart(void) {
    uint8_t buff[PN532_FRAME_MAX_LENGTH];  // buffer to store the data sent to the NFC module
    poll_count = 0;  // reset the poll count

    // Prepare the frame for starting passive target reading with specific parameters
    buff[0] = PN532_HOSTTOPN532;                          // command header
    buff[1] = PN532_COMMAND_INLISTPASSIVETARGET & 0xFF;   // command to list passive targets
    buff[2] = 0x01;                                       // parameter 1
    buff[3] = PN532_MIFARE_ISO14443A;                      // parameter 2 (MIFARE ISO14443A protocol)

    // Send the frame to the NFC module, if unsuccessful, register the callback again after 25 ms
    if (PN532_WriteFrame(&pn532, buff, 4) != PN532_ERROR_NONE) {
        eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 25, 0);
    } else {
        // If successful, start polling for NFC events (e.g., detecting the presence of a phone)
        eventRegister(nfcEventCallbackPoll, EVENT_NFC_POLL, EVENT_SINGLE, 10, 0);
    }
}

// Callback function to poll the NFC module for status
void nfcEventCallbackPoll(void) {
    uint8_t status[] = {0x00};  // buffer to store the status of the NFC module
    ++poll_count;  // increment the poll count

    // Read the status from the NFC module (polling for availability)
    if (PN532_I2C_ReadData(status, sizeof(status)) != PN532_STATUS_OK) {
#ifdef DEBUG_NFC
        // Debugging: print an error message if polling failed
        printf("[ERROR] NFC I2C poll failed\n\r");
#endif
    }

    // If the NFC module is ready, trigger the read operation
    if (status[0] == PN532_I2C_READY) {
        eventRegister(nfcEventCallbackRead, EVENT_NFC_READ, EVENT_SINGLE, 25, 0);
    } else {
        // If the NFC module is not ready, keep polling every 10 ms
        eventRegister(nfcEventCallbackPoll, EVENT_NFC_POLL, EVENT_SINGLE, 10, 0);
    }
}

// Callback function to read data from the NFC module (e.g., phone UID)
void nfcEventCallbackRead(void) {
    uint8_t buff[MIFARE_UID_MAX_LENGTH] = {0};  // buffer to store the UID data

    // Read the NFC frame from the module
    uint32_t frame_length = PN532_ReadFrame(&pn532, buff, 4);

    // If the frame does not match the expected format, handle as an error
    if (!(buff[0] == PN532_PN532TOHOST && buff[1] == (PN532_COMMAND_INLISTPASSIVETARGET + 1))) {
        // Error state: should handle restart or reset logic here
    }

    // If the frame length is valid, check if the phone is present
    if (frame_length - 2 >= 0) {
        // Insert flag indicating phone is present
        stateInsertFlag(SFLAG_NFC_PHONE_PRESENT);
    } else {
        // Insert flag indicating phone is not present
        stateInsertFlag(SFLAG_NFC_PHONE_NOT_PRESENT);
    }

    // We keep scheduling the start event to ensure the phone presence is checked periodically
    eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 25, 0);
}
}
