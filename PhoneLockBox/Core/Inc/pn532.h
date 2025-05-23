/*
 * pn532.h
 *
 *  Created on: Apr 3, 2025
 *      Author: colinriker
 */

#ifndef INC_PN532_H_
#define INC_PN532_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

#define PN532_FRAME_MAX_LENGTH 255
#define PN532_DEFAULT_TIMEOUT 1000

#define PN532_PREAMBLE                      (0x00)
#define PN532_STARTCODE1                    (0x00)
#define PN532_STARTCODE2                    (0xFF)
#define PN532_POSTAMBLE                     (0x00)

#define PN532_HOSTTOPN532                   (0xD4)
#define PN532_PN532TOHOST                   (0xD5)

// PN532 Commands
#define PN532_COMMAND_DIAGNOSE              (0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_GETGENERALSTATUS      (0x04)
#define PN532_COMMAND_READREGISTER          (0x06)
#define PN532_COMMAND_WRITEREGISTER         (0x08)
#define PN532_COMMAND_READGPIO              (0x0C)
#define PN532_COMMAND_WRITEGPIO             (0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE     (0x10)
#define PN532_COMMAND_SETPARAMETERS         (0x12)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_POWERDOWN             (0x16)
#define PN532_COMMAND_RFCONFIGURATION       (0x32)
#define PN532_COMMAND_RFREGULATIONTEST      (0x58)
#define PN532_COMMAND_INJUMPFORDEP          (0x56)
#define PN532_COMMAND_INJUMPFORPSL          (0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_COMMAND_INATR                 (0x50)
#define PN532_COMMAND_INPSL                 (0x4E)
#define PN532_COMMAND_INDATAEXCHANGE        (0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU     (0x42)
#define PN532_COMMAND_INDESELECT            (0x44)
#define PN532_COMMAND_INRELEASE             (0x52)
#define PN532_COMMAND_INSELECT              (0x54)
#define PN532_COMMAND_INAUTOPOLL            (0x60)
#define PN532_COMMAND_TGINITASTARGET        (0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES     (0x92)
#define PN532_COMMAND_TGGETDATA             (0x86)
#define PN532_COMMAND_TGSETDATA             (0x8E)
#define PN532_COMMAND_TGSETMETADATA         (0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS     (0x8A)

#define PN532_RESPONSE_INDATAEXCHANGE       (0x41)
#define PN532_RESPONSE_INLISTPASSIVETARGET  (0x4B)

#define PN532_WAKEUP                        (0x55)

#define PN532_I2C_ADDRESS                   (0x48 >> 1)
#define PN532_I2C_READBIT                   (0x01)
#define PN532_I2C_BUSY                      (0x00)
#define PN532_I2C_READY                     (0x01)
#define PN532_I2C_READYTIMEOUT              (20)

#define PN532_MIFARE_ISO14443A              (0x00)

// Mifare Commands
#define MIFARE_CMD_AUTH_A                   (0x60)
#define MIFARE_CMD_AUTH_B                   (0x61)
#define MIFARE_CMD_READ                     (0x30)
#define MIFARE_CMD_WRITE                    (0xA0)
#define MIFARE_CMD_TRANSFER                 (0xB0)
#define MIFARE_CMD_DECREMENT                (0xC0)
#define MIFARE_CMD_INCREMENT                (0xC1)
#define MIFARE_CMD_STORE                    (0xC2)
#define MIFARE_ULTRALIGHT_CMD_WRITE         (0xA2)

#define MIFARE_UID_MAX_LENGTH               MIFARE_UID_TRIPLE_LENGTH
#define MIFARE_UID_SINGLE_LENGTH            (4)
#define MIFARE_UID_DOUBLE_LENGTH            (7)
#define MIFARE_UID_TRIPLE_LENGTH            (10)
#define MIFARE_KEY_LENGTH                   (6)
#define MIFARE_BLOCK_LENGTH                 (16)


#define PN532_GPIO_VALIDATIONBIT            (0x80)

/* Official PN532 Errors Definitions */
#define PN532_ERROR_NONE                                                (0x00)
// Time Out, the target has not answered
#define PN532_ERROR_TIMEOUT                                             (0x01)
// A CRC error has been detected by the CIU
#define PN532_ERROR_CRC                                                 (0x02)
// A Parity error has been detected by the CIU
#define PN532_ERROR_PARITY                                              (0x03)
// During an anti-collision/select operation (ISO/IEC14443-3 Type A and
// ISO/IEC18092 106 kbps passive mode), an erroneous Bit Count has been
// detected
#define PN532_ERROR_COLLISION_BITCOUNT                                  (0x04)
// Framing error during MIFARE operation
#define PN532_ERROR_MIFARE_FRAMING                                      (0x05)
// An abnormal bit-collision has been detected during bit wise
// anti-collision at 106 kbps
#define PN532_ERROR_COLLISION_BITCOLLISION                              (0x06)
// Communication buffer size insufficien
#define PN532_ERROR_NOBUFS                                              (0x07)
// RF Buffer overflow has been detected by the CI
#define PN532_ERROR_RFNOBUFS                                            (0x09)
// In active communication mode, the RF field has not been switched on
// in time by the counterpart (as defined in NFCIP-1 standard
#define PN532_ERROR_ACTIVE_TOOSLOW                                      (0x0a)
// RF Protocol error
#define PN532_ERROR_RFPROTO                                             (0x0b)
// Temperature error: the internal temperature sensor has detected
// overheating, and therefore has automatically switched off the
// antenna drivers
#define PN532_ERROR_TOOHOT                                              (0x0d)
// Internal buffer overflow
#define PN532_ERROR_INTERNAL_NOBUFS                                     (0x0e)
// Invalid parameter (range, format...)
#define PN532_ERROR_INVAL                                               (0x10)
// DEP Protocol: The PN533 configured in target mode does not support
// the command received from the initiator (the command received is not
// one of the following: ATR_REQ, WUP_REQ, PSL_REQ, DEP_REQ, DSL_REQ,
// RLS_REQ)
#define PN532_ERROR_DEP_INVALID_COMMAND                                 (0x12)
// DEP Protocol, MIFARE or ISO/IEC14443-4: The data format does not
// match to the specification.  Depending on the RF protocol used, it
// can be: Bad length of RF received frame, Incorrect value of PCB or
// PFB, Invalid or unexpected RF received frame, NAD or DID incoherence.
#define PN532_ERROR_DEP_BADDATA                                         (0x13)
// MIFARE: Authentication error
#define PN532_ERROR_MIFARE_AUTH                                         (0x14)
// Target or Initiator does not support NFC Secur
#define PN532_ERROR_NOSECURE                                            (0x18)
// I2C bus line is Busy. A TDA transaction is on going
#define PN532_ERROR_I2CBUSY                                             (0x19)
// ISO/IEC14443-3: UID Check byte is wrong
#define PN532_ERROR_UIDCHECKSUM                                         (0x23)
// DEP Protocol: Invalid device state, the system is in a state which
// does not allow the operation
#define PN532_ERROR_DEPSTATE                                            (0x25)
// Operation not allowed in this configuration (host controller
// interface)
#define PN532_ERROR_HCIINVAL                                            (0x26)
// This command is not acceptable due to the current context of the
// PN533 (Initiator vs. Target, unknown target number, Target not in the
// good state, ...)
#define PN532_ERROR_CONTEXT                                             (0x27)
// The PN533 configured as target has been released by its initiator
#define PN532_ERROR_RELEASED                                            (0x29)
// PN533 and ISO/IEC14443-3B only: the ID of the card does not match,
// meaning that the expected card has been exchanged with another one.
#define PN532_ERROR_CARDSWAPPED                                         (0x2a)
// PN533 and ISO/IEC14443-3B only: the card previously activated has
// disappeared.
#define PN532_ERROR_NOCARD                                              (0x2b)
// Mismatch between the NFCID3 initiator and the NFCID3 target in DEP
// 212/424 kbps passive.
#define PN532_ERROR_MISMATCH                                            (0x2c)
// An over-current event has been detected
#define PN532_ERROR_OVERCURRENT                                         (0x2d)
// NAD missing in DEP frame
#define PN532_ERROR_NONAD                                               (0x2e)
/* End: Official PN532 Errors Definitions */

// Other Error Definitions
#define PN532_STATUS_ERROR                                              (-1)
#define PN532_STATUS_OK                                                 (0)

typedef struct _PN532 {
	int (*reset)(void);
	int (*read_data)(uint8_t* data, uint16_t count);
	int (*write_data)(uint8_t *data, uint16_t count);
	bool (*wait_ready)(uint32_t timeout);
	int (*wakeup)(void);
	void (*log)(const char* log);
} PN532;

//Setup & Util Functions
void PN532_Init(PN532* dev);
int PN532_Reset(void);
void PN532_Log(const char* log);

// Comm Functions
int PN532_I2C_ReadData(uint8_t* data, uint16_t count);
int PN532_I2C_WriteData(uint8_t *data, uint16_t count);
bool PN532_I2C_WaitReady(uint32_t timeout);
int PN532_I2C_Wakeup(void);
void PN532_I2C_Init(PN532* dev);

// NFC/PN532 funcs
int PN532_WriteFrame(PN532* pn532, uint8_t* data, uint16_t length);
int PN532_ReadFrame(PN532* pn532, uint8_t* buff, uint16_t length);
int PN532_CallFunction(PN532* pn532, uint8_t command, uint8_t* response, uint16_t response_length, uint8_t* params, uint16_t params_length, uint32_t timeout);
int PN532_GetFirmwareVersion(PN532* pn532, uint8_t* version);
int PN532_SamConfiguration(PN532* pn532);
int PN532_ReadPassiveTarget(PN532* pn532, uint8_t* response, uint8_t card_baud, uint32_t timeout);
int PN532_Ntag2xxReadBlock(PN532* pn532, uint8_t* response, uint16_t block_number);
int PN532_Ntag2xxWriteBlock(PN532* pn532, uint8_t* data, uint16_t block_number);
int PN532_ReadGpio(PN532* pn532, uint8_t* pins_state);
bool PN532_ReadGpioP(PN532* pn532, uint8_t pin_number);
bool PN532_ReadGpioI(PN532* pn532, uint8_t pin_number);
int PN532_WriteGpio(PN532* pn532, uint8_t* pins_state);
int PN532_WriteGpioP(PN532* pn532, uint8_t pin_number, bool pin_state);


#endif /* INC_PN532_H_ */
