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
 */

#include <stdlib.h>
#include <stdbool.h>

#include "lpp.h"


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
 * @brief Add an RSSI value (disguised as an "analog value" packet) from a buoy
 *        with a specific ID to the LORA packet
 *
 * @param b The pointer to the LPP pointer
 * @param id The ID of the buoy
 * @param rssi The RSSI value of the buoy
 *****************************************************************************/
bool LPP_AddBuoy(LPP_Buffer_t *b, uint8_t id, uint16_t rssi)
{
	/* Dereference operator: foo->bar is equivalent to (*foo).bar,
	 * it gets the member called bar from the struct that foo points to.*/
	uint8_t space = b->length - b->fill;
	if(space < LPP_ANALOG_INPUT_SIZE){
		return (false);
	}

	b->buffer[b->fill++] = id;
	b->buffer[b->fill++] = LPP_ANALOG_INPUT;
	b->buffer[b->fill++] = (uint8_t)((0xFF00 & rssi) >> 8);
	b->buffer[b->fill++] = (uint8_t)(0x00FF & rssi);

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
