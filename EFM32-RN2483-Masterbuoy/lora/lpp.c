/*  ____  ____      _    __  __  ____ ___
 * |  _ \|  _ \    / \  |  \/  |/ ___/ _ \
 * | | | | |_) |  / _ \ | |\/| | |  | | | |
 * | |_| |  _ <  / ___ \| |  | | |__| |_| |
 * |____/|_| \_\/_/   \_\_|  |_|\____\___/
 *                           research group
 *                             dramco.be/
 *
 *  KU Leuven - Technology Campus Gent,
 *  Gebroeders De Smetstraat 1,
 *  B-9000 Gent, Belgium
 *
 *         File: lpp.c
 *      Created: 2018-03-23
 *       Author: Geoffrey Ottoy
 *
 *  Description: Basic Low Power Payload (LPP) functionality.
 *
 *  Code added by Brecht Van Eeckhoudt.
 *
 */

#include <stdlib.h>
#include <stdbool.h>

#include "lpp.h"

/* BEGIN ADDED CODE *****************************************************************************************/
#include "dbprint.h"
/* END ADDED CODE *******************************************************************************************/


bool LPP_InitBuffer(LPP_Buffer_t *b, uint8_t size){
	LPP_ClearBuffer(b);

	b->buffer = (uint8_t *) malloc(sizeof(uint8_t) * size);

	if(b->buffer != NULL){
		b->fill = 0;
		b->length = size;
		return true;
	}

	return false;
}

void LPP_ClearBuffer(LPP_Buffer_t *b){
	if(b->buffer != NULL){
		free(b->buffer);
	}
}

bool LPP_AddDigital(LPP_Buffer_t *b, uint8_t data){
	uint8_t space = b->length - b->fill;
	if(space < LPP_DIGITAL_INPUT_SIZE){
		return false;
	}

	b->buffer[b->fill++] = LPP_DIGITAL_INPUT_CHANNEL;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = data;

	return true;
}

bool LPP_AddAnalog(LPP_Buffer_t *b, int16_t data){
	uint8_t space = b->length - b->fill;
	if(space < LPP_ANALOG_INPUT_SIZE){
		return false;
	}

	b->buffer[b->fill++] = LPP_ANALOG_INPUT_CHANNEL;
	b->buffer[b->fill++] = LPP_ANALOG_INPUT;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & data) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & data);

	return true;
}

/* BEGIN ADDED CODE *****************************************************************************************/
/**************************************************************************//**
 * @brief
 *   Add an RSSI (disguised as an "Analog Input" packet which has 2 bytes in length)
 *   and VBAT value (disguised as a "Digital Input" packet which has 1 byte in length)
 *   from a buoy with a specific ID to the LoRaWAN packet according to the
 *   "Cayenne Low Power Payload" data types.
 *
 * @note
 *   Since the Cayenne widgets can't display both the RSSI and VBAT value from a
 *   buoy (~ sensor) with the same ID (~ data channel), the VBAT value is send
 *   with an ID-offset of "1".
 *
 * @param[in] b
 *   The pointer to the LPP pointer.
 *
 * @param[in] id
 *   The ID of the buoy.
 *
 * @param[in] rssi
 *   The RSSI value of the specific buoy.
 *
 * @param[in] vbat
 *   The VBAT value of the specific buoy (gets send with an ID-offset of "1"!)
 *
 * @return
 *   @li true - Successfully added the data to the LoRaWAN packet.
 *   @li false - Coudln't add the data to the LoRaWAN packet.
 *****************************************************************************/
bool LPP_AddBuoy(LPP_Buffer_t *b, uint8_t id, uint8_t rssi, uint16_t vbat)
{
	/* Dereference operator: foo->bar is equivalent to (*foo).bar,
	 * it gets the member called bar from the struct that foo points to.
	 *   Same as line below but other notation: uint8_t space = (*b).length - (*b).fill;
	 */
	uint8_t space = b->length - b->fill;
	if(space < (LPP_ANALOG_INPUT_SIZE + LPP_DIGITAL_INPUT_SIZE))
	{
		dbprintln("CRIT: Not enough free space to add the data to the LoRaWAN packet.");
		return (false);
	}

	/* RSSI value */
	b->buffer[b->fill++] = id;
	b->buffer[b->fill++] = LPP_DIGITAL_INPUT;
	b->buffer[b->fill++] = rssi;

	/* VBAT value */
	b->buffer[b->fill++] = (id + 1);
	b->buffer[b->fill++] = LPP_ANALOG_INPUT;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & vbat) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & vbat);

	return (true);
}
/* END ADDED CODE *******************************************************************************************/

bool LPP_AddTemperature(LPP_Buffer_t *b, int16_t data){
	uint8_t space = b->length - b->fill;
	if(space < LPP_TEMPERATURE_SIZE){
		return false;
	}

	b->buffer[b->fill++] = LPP_TEMPERATURE_CHANNEL;
	b->buffer[b->fill++] = LPP_TEMPERATURE;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & data) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & data);

	return true;
}

bool LPP_AddHumidity(LPP_Buffer_t *b, uint8_t data){
	uint8_t space = b->length - b->fill;
	if(space < LPP_HUMIDITY_SIZE){
		return false;
	}

	b->buffer[b->fill++] = LPP_HUMIDITY_CHANNEL;
	b->buffer[b->fill++] = LPP_HUMIDITY;
	b->buffer[b->fill++] = data;

	return true;
}

bool LPP_AddAccelerometer(LPP_Buffer_t *b, int16_t x, int16_t y, int16_t z){
	uint8_t space = b->length - b->fill;
	if(space < LPP_ACCELEROMETER_SIZE){
		return false;
	}

	b->buffer[b->fill++] = LPP_ACCELEROMETER_CHANNEL;
	b->buffer[b->fill++] = LPP_ACCELEROMETER;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & x) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & x);
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & y) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & y);
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & z) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & z);

	return true;
}

bool LPP_AddPressure(LPP_Buffer_t *b, uint16_t data){
	uint8_t space = b->length - b->fill;
	if(space < LPP_PRESSURE_SIZE){
		return false;
	}

	b->buffer[b->fill++] = LPP_PRESSURE_CHANNEL;
	b->buffer[b->fill++] = LPP_PRESSURE;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & data) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & data);

	return true;
}
