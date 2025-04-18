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

    printf("[DEBUG NFC] Initializing PN532\n\r");

    // Initialize the I2C interface
    PN532_I2C_Init(&pn532);

    // Check if we can communicate by getting firmware version
    if (PN532_GetFirmwareVersion(&pn532, buff) == PN532_STATUS_OK) {
        printf("[DEBUG NFC] Found PN532 with firmware version: %d.%d\r\n", buff[1], buff[2]);

        // Configure the PN532
        if (PN532_SamConfiguration(&pn532) == PN532_STATUS_OK) {
            printf("[DEBUG NFC] PN532 configured successfully\n\r");
        } else {
            printf("[DEBUG NFC] Error configuring PN532\n\r");
        }
    } else {
        printf("[DEBUG NFC] Error: PN532 not responding\r\n");
    }

    // Wake up the module
    pn532.wakeup();

    // Initialize poll counter
    poll_count = 0;
    testPN532Communication();
}

void nfcEventCallbackStart(void) {
    static uint8_t reset_counter = 0;
    uint8_t buff[PN532_FRAME_MAX_LENGTH];
    poll_count = 0;

    printf("[DEBUG NFC] Starting NFC read cycle\n\r");

    // Every 5 cycles, try to reset and reinitialize the PN532
    if (reset_counter % 5 == 0) {
        printf("[DEBUG NFC] Performing periodic reset of PN532\n\r");

        // Toggle the reset pin
        HAL_GPIO_WritePin(PN532_RST_GPIO_Port, PN532_RST_Pin, GPIO_PIN_RESET);
        // Schedule a callback to set it high again
        eventRegister(nfcResetPinHigh, EVENT_NFC_START_READ, EVENT_SINGLE, 50, 0);
        reset_counter++;
        return;
    }

    reset_counter++;

    // Toggle request line
    HAL_GPIO_WritePin(PN532_REQ_GPIO_Port, PN532_REQ_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PN532_REQ_GPIO_Port, PN532_REQ_Pin, GPIO_PIN_SET);

    // Prepare command to detect passive targets
    buff[0] = PN532_HOSTTOPN532;
    buff[1] = PN532_COMMAND_INLISTPASSIVETARGET & 0xFF;
    buff[2] = 0x01; // Max number of targets to look for (1)
    buff[3] = PN532_MIFARE_ISO14443A; // Card baud rate (ISO14443A)

    int result = PN532_WriteFrame(&pn532, buff, 4);
    if (result != PN532_STATUS_OK) {
        printf("[DEBUG NFC] Error writing command frame: %d\n\r", result);

        // Retry after a delay
        eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 100, 0);
    } else {
        printf("[DEBUG NFC] Command sent successfully, scheduling poll\n\r");
        // Move to polling phase
        eventRegister(nfcEventCallbackPoll, EVENT_NFC_POLL, EVENT_SINGLE, 20, 0);
    }
}

// New function to handle the second part of the reset sequence
void nfcResetPinHigh(void) {
    printf("[DEBUG NFC] Setting PN532 reset pin high\n\r");
    HAL_GPIO_WritePin(PN532_RST_GPIO_Port, PN532_RST_Pin, GPIO_PIN_SET);

    // Schedule initialization
    eventRegister(nfcInitSequence, EVENT_NFC_START_READ, EVENT_SINGLE, 50, 0);
}

// New function to handle initialization after reset
void nfcInitSequence(void) {
    uint8_t buff[255];

    printf("[DEBUG NFC] Running PN532 initialization sequence\n\r");

    // Check if we can communicate by getting firmware version
    if (PN532_GetFirmwareVersion(&pn532, buff) == PN532_STATUS_OK) {
        printf("[DEBUG NFC] Found PN532 with firmware version: %d.%d\r\n", buff[1], buff[2]);

        // Configure the PN532
        if (PN532_SamConfiguration(&pn532) == PN532_STATUS_OK) {
            printf("[DEBUG NFC] PN532 configured successfully\n\r");
        } else {
            printf("[DEBUG NFC] Error configuring PN532\n\r");
        }
    } else {
        printf("[DEBUG NFC] Error: PN532 not responding\r\n");
    }

    // Continue with detection cycle
    eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 50, 0);
}

void nfcEventCallbackPoll(void) {
    uint8_t status[1] = {0x00};

    poll_count++;
    printf("[DEBUG NFC] Polling attempt %d, status before read: 0x%02X\n\r", poll_count, status[0]);

    // Read the status byte from PN532
    int read_result = PN532_I2C_ReadData(status, sizeof(status));
    if (read_result != PN532_STATUS_OK) {
        printf("[DEBUG NFC] Error reading status byte\n\r");

        // If we've been polling too long, restart the process
        if (poll_count >= MAX_POLL) {
            printf("[DEBUG NFC] Max poll attempts reached, restarting\n\r");
            stateInsertFlag(SFLAG_NFC_PHONE_NOT_PRESENT);

            // Restart from the beginning with a longer delay
            eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 100, 0);
        } else {
            // Continue polling with increasing delays (backoff strategy)
            uint16_t delay = 10 + (poll_count * 5); // Increasing delay with each attempt
            eventRegister(nfcEventCallbackPoll, EVENT_NFC_POLL, EVENT_SINGLE, delay, 0);
        }
        return;
    }

    printf("[DEBUG NFC] Status after read: 0x%02X\n\r", status[0]);

    if (status[0] == PN532_I2C_READY) {
        printf("[DEBUG NFC] PN532 is ready with data\n\r");
        eventRegister(nfcEventCallbackRead, EVENT_NFC_READ, EVENT_SINGLE, 10, 0);
    } else {
        // Not ready yet

        // If we've been polling too long, restart the process
        if (poll_count >= MAX_POLL) {
            printf("[DEBUG NFC] Max poll attempts reached, restarting\n\r");
            stateInsertFlag(SFLAG_NFC_PHONE_NOT_PRESENT);
            printf("[DEBUG NFC] Set SFLAG_NFC_PHONE_NOT_PRESENT (timeout)\n\r");

            // Restart from the beginning
            eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 100, 0);
        } else {
            // Continue polling with increasing delays
            uint16_t delay = 10 + (poll_count * 5);
            eventRegister(nfcEventCallbackPoll, EVENT_NFC_POLL, EVENT_SINGLE, delay, 0);
        }
    }
}

void nfcEventCallbackRead(void) {
    uint8_t buff[PN532_FRAME_MAX_LENGTH] = {0}; // Use proper buffer size

    printf("[DEBUG NFC] Reading frame data\n\r");
    int32_t frame_length = PN532_ReadFrame(&pn532, buff, sizeof(buff));

    printf("[DEBUG NFC] Frame length: %ld\n\r", frame_length);

    // Check for errors
    if (frame_length == PN532_STATUS_ERROR) {
        printf("[DEBUG NFC] Error reading frame\n\r");
        stateInsertFlag(SFLAG_NFC_PHONE_NOT_PRESENT);
        printf("[DEBUG NFC] Set SFLAG_NFC_PHONE_NOT_PRESENT (read error)\n\r");
        eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 100, 0);
        return;
    }

    // Check basic frame format
    if (frame_length < 2) {
        printf("[DEBUG NFC] Frame too short\n\r");
        stateInsertFlag(SFLAG_NFC_PHONE_NOT_PRESENT);
        printf("[DEBUG NFC] Set SFLAG_NFC_PHONE_NOT_PRESENT (short frame)\n\r");
        eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 100, 0);
        return;
    }

    printf("[DEBUG NFC] Header bytes: 0x%02X 0x%02X\n\r", buff[0], buff[1]);

    // Check for proper response format
    if (!(buff[0] == PN532_PN532TOHOST && buff[1] == (PN532_COMMAND_INLISTPASSIVETARGET + 1))) {
        printf("[DEBUG NFC] Invalid response format\n\r");
        stateInsertFlag(SFLAG_NFC_PHONE_NOT_PRESENT);
        printf("[DEBUG NFC] Set SFLAG_NFC_PHONE_NOT_PRESENT (invalid format)\n\r");
        eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 100, 0);
        return;
    }

    // Check if a target was found (buff[2] contains number of targets)
    if (frame_length > 2 && buff[2] > 0) {
        printf("[DEBUG NFC] Tag detected! Number of targets: %d\n\r", buff[2]);

        stateInsertFlag(SFLAG_NFC_PHONE_PRESENT);
        printf("[DEBUG NFC] Set SFLAG_NFC_PHONE_PRESENT\n\r");

        // Print UID if available
        if (frame_length > 10) { // We have enough data for a UID
            printf("[DEBUG NFC] Tag UID: ");
            for (int i = 0; i < buff[7]; i++) {
                printf("%02X ", buff[8+i]);
            }
            printf("\n\r");
        }
    } else {
        printf("[DEBUG NFC] No tag detected\n\r");
        stateInsertFlag(SFLAG_NFC_PHONE_NOT_PRESENT);
        printf("[DEBUG NFC] Set SFLAG_NFC_PHONE_NOT_PRESENT\n\r");
    }

    // Schedule the next read cycle with a small delay
    eventRegister(nfcEventCallbackStart, EVENT_NFC_START_READ, EVENT_SINGLE, 100, 0);
}

bool testPN532Communication(void) {
    printf("[TEST] Attempting basic PN532 communication test\n\r");

    // Reset the module using the existing function
    printf("[TEST] Resetting PN532 hardware\n\r");
    PN532_Reset();

    // Try to wake up the module
    printf("[TEST] Waking up PN532\n\r");
    pn532.wakeup();

    // Try to read firmware version
    uint8_t version[4];
    printf("[TEST] Reading firmware version\n\r");
    int status = PN532_GetFirmwareVersion(&pn532, version);

    if (status == PN532_STATUS_OK) {
        printf("[TEST] PN532 communication successful - version %d.%d\n\r",
               version[1], version[2]);
        return true;
    } else {
        printf("[TEST] PN532 communication failed - status: %d\n\r", status);
        return false;
    }
}
