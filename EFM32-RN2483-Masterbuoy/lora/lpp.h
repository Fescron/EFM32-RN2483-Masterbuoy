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
 *         File: lpp.h
 *      Created: 2018-03-23
 *       Author: Geoffrey Ottoy
 *
 *  Description: Header file for lpp.c
 *
 *  Code added by Brecht Van Eeckhoudt.
 *
 */


#ifndef _LPP_H_
#define _LPP_H_

#include <em_device.h>
#include <stdbool.h>


// lpp types
#define LPP_DIGITAL_INPUT			0x00
#define LPP_ANALOG_INPUT			0x02
#define LPP_TEMPERATURE				0x67
#define LPP_HUMIDITY				0x68
#define LPP_ACCELEROMETER			0x71
#define LPP_PRESSURE				0x73

// lpp data sizes
#define LPP_DIGITAL_INPUT_SIZE		0x03
#define LPP_ANALOG_INPUT_SIZE		0x04
#define LPP_TEMPERATURE_SIZE		0x04
#define LPP_HUMIDITY_SIZE			0x03
#define LPP_ACCELEROMETER_SIZE		0x08
#define LPP_PRESSURE_SIZE			0x04

// lpp channel ids
#define LPP_DIGITAL_INPUT_CHANNEL	0x01
#define LPP_ANALOG_INPUT_CHANNEL	0x02
#define LPP_TEMPERATURE_CHANNEL		0x03
#define LPP_HUMIDITY_CHANNEL		0x04
#define LPP_ACCELEROMETER_CHANNEL	0x05
#define LPP_PRESSURE_CHANNEL		0x06


typedef struct lpp_buffer{
	uint8_t * buffer;
	uint8_t fill;
	uint8_t length;
} LPP_Buffer_t;

bool LPP_InitBuffer(LPP_Buffer_t * b, uint8_t size);
void LPP_ClearBuffer(LPP_Buffer_t * b);

bool LPP_AddDigital(LPP_Buffer_t *b, uint8_t data);
bool LPP_AddAnalog(LPP_Buffer_t *b, int16_t data);
bool LPP_AddTemperature(LPP_Buffer_t *b, int16_t data);
bool LPP_AddHumidity(LPP_Buffer_t *b, uint8_t data);
bool LPP_AddAccelerometer(LPP_Buffer_t *b, int16_t x, int16_t y, int16_t z);
bool LPP_AddPressure(LPP_Buffer_t *b, uint16_t data);

/* BEGIN ADDED CODE *****************************************************************************************/
bool LPP_AddBuoy(LPP_Buffer_t *b, uint8_t id, uint8_t rssi, uint16_t vbat);
/* END ADDED CODE *******************************************************************************************/

#endif /* _LPP_H_ */
