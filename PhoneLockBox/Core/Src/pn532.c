/**************************************************************************
 *  @file     pn532_stm32f1.c
 *  @author   Yehui from Waveshare
 *  @license  BSD
 *
 *  This implements the peripheral interfaces.
 *
 *  Check out the links above for our tutorials and wiring diagrams
 *  These chips use SPI communicate.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 * Modifications Made by Colin Riker, colinrik@umich.edu, University of Michigan
 *
 **************************************************************************/
#include <stdio.h>
#include <stdbool.h>

#include "pn532.h"

#include "stm32l4xx_hal.h"

const uint8_t PN532_ACK[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
const uint8_t PN532_FRAME_START[] = {0x00, 0x00, 0xFF};

#define _I2C_ADDRESS 0x48
#define _I2C_TIMEOUT 10


extern I2C_HandleTypeDef hi2c1;



/**
 * @brief: Write a frame to the PN532 of at most length bytes in size.
 *     Note that less than length bytes might be returned!
 * @retval: Returns -1 if there is an error parsing the frame.
 */
int PN532_WriteFrame(PN532* pn532, uint8_t* data, uint16_t length) {
	if (length > PN532_FRAME_MAX_LENGTH || length < 1) {
		return PN532_STATUS_ERROR; // Data must be array of 1 to 255 bytes.
	}
	// Build frame to send as:
	// - Preamble (0x00)
	// - Start code  (0x00, 0xFF)
	// - Command length (1 byte)
	// - Command length checksum
	// - Command bytes
	// - Checksum
	// - Postamble (0x00)

	uint8_t frame[PN532_FRAME_MAX_LENGTH + 7];
	uint8_t checksum = 0;
	frame[0] = PN532_PREAMBLE;
	frame[1] = PN532_STARTCODE1;
	frame[2] = PN532_STARTCODE2;
	for (uint8_t i = 0; i < 3; i++) {
		checksum += frame[i];
	}
	frame[3] = length & 0xFF;
	frame[4] = (~length + 1) & 0xFF;
	for (uint8_t i = 0; i < length; i++) {
		frame[5 + i] = data[i];
		checksum += data[i];
	}
	frame[length + 5] = ~checksum & 0xFF;
	frame[length + 6] = PN532_POSTAMBLE;
	if (pn532->write_data(frame, length + 7) != PN532_STATUS_OK) {
		return PN532_STATUS_ERROR;
	}
	return PN532_STATUS_OK;
}

/**
 * @brief: Read a response frame from the PN532 of at most length bytes in size.
 *     Note that less than length bytes might be returned!
 * @retval: Returns frame length or -1 if there is an error parsing the frame.
 */
int PN532_ReadFrame(PN532* pn532, uint8_t* response, uint16_t length) {
	uint8_t buff[PN532_FRAME_MAX_LENGTH + 7];
	uint8_t checksum = 0;
	// Read frame with expected length of data.
	pn532->read_data(buff, length + 7);
	// Swallow all the 0x00 values that preceed 0xFF.
	uint8_t offset = 0;
	while (buff[offset] == 0x00) {
		offset += 1;
		if (offset >= length + 8){
#ifdef DEBUG_NFC
			pn532->log("Response frame preamble does not contain 0x00FF!");
#endif
			return PN532_STATUS_ERROR;
		}
	}
	if (buff[offset] != 0xFF) {
#ifdef DEBUG_NFC
		pn532->log("Response frame preamble does not contain 0x00FF!");
#endif
		return PN532_STATUS_ERROR;
	}
	offset += 1;
	if (offset >= length + 8) {
#ifdef DEBUG_NFC
		pn532->log("Response contains no data!");
#endif
		return PN532_STATUS_ERROR;
	}
	// Check length & length checksum match.
	uint8_t frame_len = buff[offset];
	if (((frame_len + buff[offset+1]) & 0xFF) != 0) {
#ifdef DEBUG_NFC
		pn532->log("Response length checksum did not match length!");
#endif
		return PN532_STATUS_ERROR;
	}
	// Check frame checksum value matches bytes.
	for (uint8_t i = 0; i < frame_len + 1; i++) {
		checksum += buff[offset + 2 + i];
	}
	checksum &= 0xFF;
	if (checksum != 0) {
#ifdef DEBUG_NFC
		pn532->log("Response checksum did not match expected checksum");
#endif
		return PN532_STATUS_ERROR;
	}
	// Return frame data.
	for (uint8_t i = 0; i < frame_len; i++) {
		response[i] = buff[offset + 2 + i];
	}
	return frame_len;
}

/**
 * @brief: Send specified command to the PN532 and expect up to response_length.
 *     Will wait up to timeout seconds for a response and read a bytearray into
 *     response buffer.
 * @param pn532: PN532 handler
 * @param command: command to send
 * @param response: buffer returned
 * @param response_length: expected response length
 * @param params: can optionally specify an array of bytes to send as parameters
 *     to the function call, or NULL if there is no need to send parameters.
 * @param params_length: length of the argument params
 * @param timeout: timout of systick
 * @retval: Returns the length of response or -1 if error.
 */
int PN532_CallFunction(
		PN532* pn532,
		uint8_t command,
		uint8_t* response,
		uint16_t response_length,
		uint8_t* params,
		uint16_t params_length,
		uint32_t timeout
) {
	// Build frame data with command and parameters.
	uint8_t buff[PN532_FRAME_MAX_LENGTH];
	buff[0] = PN532_HOSTTOPN532;
	buff[1] = command & 0xFF;
	for (uint8_t i = 0; i < params_length; i++) {
		buff[2 + i] = params[i];
	}
	// Send frame and wait for response.
	if (PN532_WriteFrame(pn532, buff, params_length + 2) != PN532_STATUS_OK) {
		pn532->wakeup();
#ifdef DEBUG_NFC
		pn532->log("Trying to wakeup");
#endif
		return PN532_STATUS_ERROR;
	}
	if (!pn532->wait_ready(timeout)) {
		return PN532_STATUS_ERROR;
	}
	// Verify ACK response and wait to be ready for function response.
	pn532->read_data(buff, sizeof(PN532_ACK));
	for (uint8_t i = 0; i < sizeof(PN532_ACK); i++) {
		if (PN532_ACK[i] != buff[i]) {
#ifdef DEBUG_NFC
			pn532->log("Did not receive expected ACK from PN532!");
#endif
			return PN532_STATUS_ERROR;
		}
	}
	if (!pn532->wait_ready(timeout)) {
		return PN532_STATUS_ERROR;
	}
	// Read response bytes.
	int frame_len = PN532_ReadFrame(pn532, buff, response_length + 2);

	// Check that response is for the called function.
	if (! ((buff[0] == PN532_PN532TOHOST) && (buff[1] == (command+1)))) {
#ifdef DEBUG_NFC
		pn532->log("Received unexpected command response!");
#endif
		return PN532_STATUS_ERROR;
	}
	// Return response data.
	for (uint8_t i = 0; i < response_length; i++) {
		response[i] = buff[i + 2];
	}
	// The the number of bytes read
	return frame_len - 2;
}

/**
 * @brief: Call PN532 GetFirmwareVersion function and return a buff with the IC,
 *  Ver, Rev, and Support values.
 */
int PN532_GetFirmwareVersion(PN532* pn532, uint8_t* version) {
	// length of version: 4
	if (PN532_CallFunction(pn532, PN532_COMMAND_GETFIRMWAREVERSION,
			version, 4, NULL, 0, 500) == PN532_STATUS_ERROR) {
#ifdef DEBUG_NFC
		pn532->log("Failed to detect the PN532");
#endif
		return PN532_STATUS_ERROR;
	}
	return PN532_STATUS_OK;
}

/**
 * @brief: Configure the PN532 to read MiFare cards.
 */
int PN532_SamConfiguration(PN532* pn532) {
	// Send SAM configuration command with configuration for:
	// - 0x01, normal mode
	// - 0x14, timeout 50ms * 20 = 1 second
	// - 0x01, use IRQ pin
	// Note that no other verification is necessary as call_function will
	// check the command was executed as expected.
	uint8_t params[] = {0x01, 0x14, 0x01};
	PN532_CallFunction(pn532, PN532_COMMAND_SAMCONFIGURATION,
			NULL, 0, params, sizeof(params), PN532_DEFAULT_TIMEOUT);
	return PN532_STATUS_OK;
}

/**
 * @brief: Wait for a MiFare card to be available and return its UID when found.
 *     Will wait up to timeout seconds and return None if no card is found,
 *     otherwise a bytearray with the UID of the found card is returned.
 * @retval: Length of UID, or -1 if error.
 */
int PN532_ReadPassiveTarget(
		PN532* pn532,
		uint8_t* response,
		uint8_t card_baud,
		uint32_t timeout
) {
	// Send passive read command for 1 card.  Expect at most a 7 byte UUID.
	uint8_t params[] = {0x01, card_baud};
	uint8_t buff[19];
	int length = PN532_CallFunction(pn532, PN532_COMMAND_INLISTPASSIVETARGET,
			buff, sizeof(buff), params, sizeof(params), timeout);
	if (length < 0) {
		return PN532_STATUS_ERROR; // No card found
	}
	// Check only 1 card with up to a 7 byte UID is present.
	if (buff[0] != 0x01) {
#ifdef DEBUG_NFC
		pn532->log("More than one card detected!");
#endif
		return PN532_STATUS_ERROR;
	}
	if (buff[5] > 7) {
#ifdef DEBUG_NFC
		pn532->log("Found card with unexpectedly long UID!");
#endif
		return PN532_STATUS_ERROR;
	}
	for (uint8_t i = 0; i < buff[5]; i++) {
		response[i] = buff[6 + i];
	}
	return buff[5];
}


int PN532_ReadGpio(PN532* pn532, uint8_t* pins_state) {
	return PN532_CallFunction(pn532, PN532_COMMAND_READGPIO, pins_state, 3,
			NULL, 0, PN532_DEFAULT_TIMEOUT);
}
/**
 * @brief: Read the GPIO state of specified pins in (P30 ... P35).
 * @param pin_number: specify the pin to read.
 * @retval: true if HIGH, false if LOW
 */
bool PN532_ReadGpioP(PN532* pn532, uint8_t pin_number) {
	uint8_t pins_state[3];
	PN532_CallFunction(pn532, PN532_COMMAND_READGPIO, pins_state,
			sizeof(pins_state), NULL, 0, PN532_DEFAULT_TIMEOUT);
	if ((pin_number >= 30) && (pin_number <= 37)) {
		return (pins_state[0] >> (pin_number - 30)) & 1 ? true : false;
	}
	if ((pin_number >= 70) && (pin_number <= 77)) {
		return (pins_state[1] >> (pin_number - 70)) & 1 ? true : false;
	}
	return false;
}
/**
 * @brief: Read the GPIO state of I0 or I1 pin.
 * @param pin_number: specify the pin to read.
 * @retval: true if HIGH, false if LOW
 */
bool PN532_ReadGpioI(PN532* pn532, uint8_t pin_number) {
	uint8_t pins_state[3];
	PN532_CallFunction(pn532, PN532_COMMAND_READGPIO, pins_state,
			sizeof(pins_state), NULL, 0, PN532_DEFAULT_TIMEOUT);
	if (pin_number <= 7) {
		return (pins_state[2] >> pin_number) & 1 ? true : false;
	}
	return false;
}
/**
 * @brief: Write the GPIO states.
 * @param pins_state: pin state buffer (2 bytes) to write.
 *     no need to read pin states before write with the param pin_state
 *         P3 = pin_state[0], P7 = pin_state[1]
 *     bits:
 *         P3[0] = P30,   P7[0] = 0,
 *         P3[1] = P31,   P7[1] = P71,
 *         P3[2] = P32,   P7[2] = P72,
 *         P3[3] = P33,   P7[3] = nu,
 *         P3[4] = P34,   P7[4] = nu,
 *         P3[5] = P35,   P7[5] = nu,
 *         P3[6] = nu,    P7[6] = nu,
 *         P3[7] = Val,   P7[7] = Val,
 *     For each port that is validated (bit Val = 1), all the bits are applied
 *     simultaneously. It is not possible for example to modify the state of
 *     the port P32 without applying a value to the ports P30, P31, P33, P34
 *     and P35.
 * @retval: -1 if error
 */
int PN532_WriteGpio(PN532* pn532, uint8_t* pins_state) {
	uint8_t params[2];
	// 0x80, the validation bit.
	params[0] = 0x80 | pins_state[0];
	params[1] = 0x80 | pins_state[1];
	return PN532_CallFunction(pn532, PN532_COMMAND_WRITEGPIO, NULL, 0,
			params, sizeof(params), PN532_DEFAULT_TIMEOUT);
}
/**
 * @brief: Write the specified pin with given states.
 * @param pin_number: specify the pin to write.
 * @param pin_state: specify the pin state. true for HIGH, false for LOW.
 * @retval: -1 if error
 */
int PN532_WriteGpioP(PN532* pn532, uint8_t pin_number, bool pin_state) {
	uint8_t pins_state[2];
	uint8_t params[2];
	if (PN532_ReadGpio(pn532, pins_state) == PN532_STATUS_ERROR) {
		return PN532_STATUS_ERROR;
	}
	if ((pin_number >= 30) && (pin_number <= 37)) {
		if (pin_state) {
			params[0] = 0x80 | pins_state[0] | 1 << (pin_number - 30);
		} else {
			params[0] = (0x80 | pins_state[0]) & ~(1 << (pin_number - 30));
		}
		params[1] = 0x00;   // leave p7 unchanged
	}
	if ((pin_number >= 70) && (pin_number <= 77)) {
		if (pin_state) {
			params[1] = 0x80 | pins_state[1] | 1 << (pin_number - 70);
		} else {
			params[1] = (0x80 | pins_state[1]) & ~(1 << (pin_number - 70));
		}
		params[0] = 0x00;   // leave p3 unchanged
	}
	return PN532_CallFunction(pn532, PN532_COMMAND_WRITEGPIO, NULL, 0,
			params, sizeof(params), PN532_DEFAULT_TIMEOUT);
}




/**************************************************************************
 * Reset and Log implements
 **************************************************************************/
int PN532_Reset(void) {
	HAL_GPIO_WritePin(PN532_RST_GPIO_Port, PN532_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(PN532_RST_GPIO_Port, PN532_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(PN532_RST_GPIO_Port, PN532_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	return PN532_STATUS_OK;
}

void PN532_Log(const char* log) {
	printf("%s\r\n", log);
}

void PN532_Init(PN532* pn532) {
	//PN532_SPI_Init(pn532);
}
/**************************************************************************
 * End: Reset and Log implements
 **************************************************************************/

/**************************************************************************
 * I2C
 **************************************************************************/
void i2c_read(uint8_t* data, uint16_t count) {
	if (HAL_I2C_Master_Receive(&hi2c1, _I2C_ADDRESS, data, count, _I2C_TIMEOUT) != HAL_OK) {
#ifdef DEBUG_NFC
		printf("[ERROR] NFC receive I2C transmit failed\n\r");
#endif
	}
}

void i2c_write(uint8_t* data, uint16_t count) {
	if (HAL_I2C_Master_Transmit(&hi2c1, _I2C_ADDRESS, data, count, _I2C_TIMEOUT) != HAL_OK) {
#ifdef DEBUG_NFC
		printf("[ERROR] NFC receive I2C transmit failed\n\r");
#endif
	}
}

int PN532_I2C_ReadData(uint8_t* data, uint16_t count) {
	uint8_t status[] = {0x00};
	uint8_t frame[count + 1];
	i2c_read(status, sizeof(status));
	if (status[0] != PN532_I2C_READY) {
		return PN532_STATUS_ERROR;
	}
	i2c_read(frame, count + 1);
	for (uint8_t i = 0; i < count; i++) {
		data[i] = frame[i + 1];
	}
	return PN532_STATUS_OK;
}

int PN532_I2C_WriteData(uint8_t *data, uint16_t count) {
	i2c_write(data, count);
	return PN532_STATUS_OK;
}

bool PN532_I2C_WaitReady(uint32_t timeout) {
	uint8_t status[] = {0x00};
	uint32_t tickstart = HAL_GetTick();
	while (HAL_GetTick() - tickstart < timeout) {
		i2c_read(status, sizeof(status));
		if (status[0] == PN532_I2C_READY) {
			return true;
		} else {
			HAL_Delay(5);
		}
	}
	return false;
}

int PN532_I2C_Wakeup(void) {
	// TODO
	HAL_GPIO_WritePin(PN532_REQ_GPIO_Port, PN532_REQ_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(PN532_REQ_GPIO_Port, PN532_REQ_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(PN532_REQ_GPIO_Port, PN532_REQ_Pin, GPIO_PIN_SET);
	HAL_Delay(500);
	return PN532_STATUS_OK;
}

void PN532_I2C_Init(PN532* pn532) {
	// init the pn532 functions
	pn532->reset =  PN532_Reset;
	pn532->read_data = PN532_I2C_ReadData;
	pn532->write_data = PN532_I2C_WriteData;
	pn532->wait_ready = PN532_I2C_WaitReady;
	pn532->wakeup = PN532_I2C_Wakeup;
	pn532->log = PN532_Log;

	// hardware wakeup
	pn532->wakeup();
}
/**************************************************************************
 * End: I2C
 **************************************************************************/
