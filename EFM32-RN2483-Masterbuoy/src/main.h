/***************************************************************************//**
 * @file main.h
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/

/* include guards prevent multiple inclusions of the same header */
#ifndef _MAIN_H_
#define _MAIN_H_


#include <stdint.h>     /* (u)intXX_t */

typedef enum app_states{
	INIT,
	JOIN,
	MEASURE,
	SEND_MEASUREMENTS,
	SLEEP,
	DEEP_SLEEP,
	WAKE_UP
} APP_State_t;

extern volatile APP_State_t appState; /* before this was "static" aswell */


/* Global variables */

/* Buffers to parse data from, local copy just in case new data is received using interrupts */
extern volatile char receiveBuffer0[DBPRINT_BUFFER_SIZE];
extern volatile bool receiveBuffer0_used;

extern volatile char receiveBuffer1[DBPRINT_BUFFER_SIZE];
extern volatile bool receiveBuffer1_used;

extern volatile char receiveBuffer2[DBPRINT_BUFFER_SIZE];
extern volatile bool receiveBuffer2_used;


/* Parsed buffer data */
extern volatile uint8_t id0;
extern volatile uint16_t rssi0;
extern volatile bool data0_ready; /* If true: ready to pack in a LPP packet */

extern volatile uint8_t id1;
extern volatile uint16_t rssi1;
extern volatile bool data1_ready;

extern volatile uint8_t id2;
extern volatile uint16_t rssi2;
extern volatile bool data2_ready;


/* Prototypes */
void LED_ERROR (uint8_t err); /* I only need this in leuart.c so I only define this */

#endif /* _MAIN_H_ */
