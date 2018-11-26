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
 *         File: main.c
 *      Created: 2018-01-18
 *
 *  Description: Main application.
 *
 *  Code added by Brecht Van Eeckhoudt.
 *
 */

#include <stdbool.h>
#include <stdlib.h>

// LoRa Communication
#include "lora.h"
#include "lpp.h"
#include "my_lora_device.h"
// Micro-controller system
#include "system.h"
// Sensors
#include "lis3dh.h"
#include "si7021.h"
#include "bme280.h"

/* BEGIN ADDED CODE *****************************************************************************************/
/* NOTES:
 *
 *   Added code in the following files when starting from the DRAMCO example:
 *     - my_lora_device.h ("LORAWAN_DEVICE_EUI", "LORAWAN_APPLICATION_EUI" and "LORAWAN_APPLICATION_KEY" hidden)
 *     - lpp.c
 *     - lpp.h
 *     - leuart.c
 *
 *   Added the following file:
 *     - main.h
 *
 */
#include "dbprint.h" /* Homebrew printf */
#include "../src/main.h"

/* Uncomment line below to skip errors were LoRa things are not successful */
//#define SKIPLORA

/* Comment line below to route UART1 RX to PC1 (EXP HEADER pin 5) and UART1 TX to PC0 (EXP HEADER pin 3) instead of VCOM */
//#define VCOM

/* END ADDED CODE *******************************************************************************************/



/* LET SENSORS GO TO SLEEP:
 * 	- set to 1 to let sensors sleep
 * 	- set to 0 to not let sensors go to sleep
 */
#define APP_LET_SENSORS_SLEEP 0

// DO NOT TOUCH
// This functionality is adjusted through the above APP_LET_SENSORS_SLEEP parameter
#if APP_LET_SENSORS_SLEEP == 1
	#define SENSORS_OUT_SLEEP() PM_Enable(PM_SENS_GECKO)
	#define SENSORS_TO_SLEEP() 	PM_Disable(PM_SENS_GECKO)
#else
	#define SENSORS_OUT_SLEEP() PM_Enable(PM_SENS_GECKO)
	#define SENSORS_TO_SLEEP() ((void)0)
#endif

#define A_MINUTE 60000

volatile uint8_t errorNr = 0;
volatile bool wakeUp;


/* BEGIN ADDED CODE *****************************************************************************************/
volatile APP_State_t appState; /* In DRAMCO example this was "static" too */


/* Global variables */

/* Buffers to parse data from, local copy just in case new data is received using interrupts */
volatile char receiveBuffer0[DBPRINT_BUFFER_SIZE];
volatile bool receiveBuffer0_used = false;

volatile char receiveBuffer1[DBPRINT_BUFFER_SIZE];
volatile bool receiveBuffer1_used = false;

volatile char receiveBuffer2[DBPRINT_BUFFER_SIZE];
volatile bool receiveBuffer2_used = false;


/* Parsed buffer data */
volatile uint8_t id0 = 0;
volatile uint8_t rssi0 = 0;
volatile uint16_t vbat0 = 0;
volatile bool data0_ready = false; /* If true: ready to pack in a LPP packet */

volatile uint8_t id1 = 0;
volatile uint8_t rssi1 = 0;
volatile uint16_t vbat1 = 0;
volatile bool data1_ready = false;

/* END ADDED CODE *******************************************************************************************/



/* Interrupt callback functions */
// Button 1 callback
void PB1_Pressed(void){
	/* BEGIN ADDED CODE *****************************************************************************************/
	dbprintln(">> PB1_Pressed");
	/* END ADDED CODE *******************************************************************************************/
	if(appState == SLEEP){
		LED_On();
		wakeUp = true;
	}
}

// Button 0 callback
void PB0_Pressed(void){
	/* BEGIN ADDED CODE *****************************************************************************************/
	dbprintln(">> PB0_Pressed");
	/* END ADDED CODE *******************************************************************************************/
}

void LED_ERROR(uint8_t err){
	/* BEGIN ADDED CODE *****************************************************************************************/
	dbprint(">> LED_ERROR (");
	dbprintUint(err);
	dbprintln(")");
	/* END ADDED CODE *******************************************************************************************/

	errorNr = err;
	while(1){
		DelayMs(200);
		LED_Toggle();
	}
}

void LED_HALTED(void){
	/* BEGIN ADDED CODE *****************************************************************************************/
	dbprintln(">> LED_HALTED");
	/* END ADDED CODE *******************************************************************************************/
	while(1){
		DelayMs(2000);
		LED_Toggle();
	}
}

int main(void){
	/* Application variables */
	LoRaSettings_t loraSettings = LORA_INIT_MY_DEVICE;
	LoRaStatus_t loraStatus;
	LPP_Buffer_t appData;
	uint32_t rhData; /* Relative humidity */
	int32_t tData; /* Temperature */
	uint32_t batteryLevel;

	appState = INIT;

	memset(&appData, 0, sizeof(appData));

	while(1){
		switch(appState){
			case INIT:{
				/* Initialize the system hardware */
				System_Init();

				/* BEGIN ADDED CODE *****************************************************************************************/
#ifdef VCOM
				/* Initialize VCOM UART in interrupt mode */
				dbprint_INIT(USART1, 4, true, true);
#else
				/* Initialize UART in interrupt mode */
				dbprint_INIT(USART1, 0, false, true);
#endif
				dbprintln("> INIT");
				/* END ADDED CODE *******************************************************************************************/

				// Now we can do stuff like this:
					// Respond to button press (interrupt-based)
				Buttons_AttachInterrupt(&PB0_Pressed, BUTTON_PB0);
				Buttons_AttachInterrupt(&PB1_Pressed, BUTTON_PB1);
					// Read the battery level
				ADC_Measure(BATTERY_LEVEL, &batteryLevel);
					// Wait (in EM2)
				DelayMs(500);

				/* Initialize sensors */
				// 1. Temperature and relative humidity
				SENSORS_OUT_SLEEP();
				if(!Si7021_Detect()){
					LED_ERROR(1);
				}
				SENSORS_TO_SLEEP();

				appState = JOIN;
			} break;
			case JOIN:{
				/* BEGIN ADDED CODE *****************************************************************************************/
				dbprintln("> JOIN");

				/* Initialize LoRa communication */
				loraStatus = LoRa_Init(loraSettings);

#ifdef SKIPLORA
			    dbprintln("WARN: Skipped LoRa initialization for debugging");
#else
				if(loraStatus != JOINED){
					dbprintln("CRIT: Couldn't initialize LoRa communication");
					LED_ERROR(2);
				}
#endif
				/* END ADDED CODE *******************************************************************************************/
				appState = MEASURE;
			} break;
			case MEASURE:{
				/* BEGIN ADDED CODE *****************************************************************************************/
				dbprintln("> MEASURE");
				/* END ADDED CODE *******************************************************************************************/

				/* Perform measurements */
				// Measure temperature and relative humidity
				SENSORS_OUT_SLEEP();
				Si7021_MeasureRHAndTemp(&rhData, &tData);
				SENSORS_TO_SLEEP();
				// Measure battery level
				ADC_Measure(BATTERY_LEVEL, &batteryLevel);

				// No send these measurements
				appState = SEND_MEASUREMENTS;
			} break;
			case SEND_MEASUREMENTS:{
				/* BEGIN ADDED CODE *****************************************************************************************/
				dbprintln("> SEND MEASUREMENTS");

				/* Send measurement data to "the cloud" */

				// Convert sensor readings to LPP format
				//int16_t tempLPP = (int16_t)(round((float)tData/100));
				// uint8_t humidityLPP = (uint8_t)(rhData/500);
				// int16_t batteryLPP = (int16_t)(round((float)batteryLevel/10));

				if (data0_ready || data1_ready)
				{
					// Initialize and fill LPP-formatted payload
					if(!LPP_InitBuffer(&appData, 16)){
						dbprintln("CRIT: Couldn't initialize LPP packet");
						LED_ERROR(3);
					}
					/*
					if(!LPP_AddTemperature(&appData, tempLPP)){
						dbprintln("CRIT: Couldn't add temperature to LPP packet");
						LED_ERROR(4);
					}
					if(!LPP_AddHumidity(&appData, humidityLPP)){
						LED_ERROR(5);
					}
					if(!LPP_AddAnalog(&appData, batteryLPP)){
						LED_ERROR(6);
					}
					*/

					/* Add ready data to LPP packets */

					if (data0_ready)
					{
						/* Add data to packet (go to error if not successful)
						 *   NOTE: The VBAT value gets send to Cayenne with an ID-offset of "1"!
						 */
						if (!LPP_AddBuoy(&appData, id0, rssi0, vbat0))
						{
							dbprintln("CRIT: Couldn't add data0 to LPP packet");
							LED_ERROR(7);
						}

						/* Mark data buffer as free */
						data0_ready = false;

						dbprintln("INFO: data0 added to LPP packet");
					}

					if (data1_ready)
					{
						/* Add data to packet (go to error if not successful)
						 *   NOTE: The VBAT value gets send to Cayenne with an ID-offset of "1"!
						 */
						if (!LPP_AddBuoy(&appData, id1, rssi1, vbat1))
						{
							dbprintln("CRIT: Couldn't add data1 to LPP packet");
							LED_ERROR(7);
						}

						/* Mark data buffer as free */
						data1_ready = false;

						dbprintln("INFO: data1 added to LPP packet");
					}

					// Send LPP-formatted payload
					if(LoRa_SendLppBuffer(appData, LORA_UNCONFIMED) != SUCCESS){
	#ifdef SKIPLORA
						dbprintln("WARN: Couldn't send LPP-formatted payload but skipped for debugging");
	#else
						dbprintln("CRIT: Couldn't send LPP-formatted payload");
						LED_ERROR(8);
	#endif
					}
					else
					{
						dbprintln("INFO: LPP packet send successfully");
					}
				}
				else
				{
					dbprintln("INFO: No data to send");
				}

				/* END ADDED CODE *******************************************************************************************/


				// Go to sleep
				appState = SLEEP;
			} break;
			case SLEEP:{
				/* BEGIN ADDED CODE *****************************************************************************************/
				dbprintln("> SLEEP");
				/* END ADDED CODE *******************************************************************************************/

				// Sleep for a specified period of time;
				wakeUp = false;
				/* BEGIN ADDED CODE *****************************************************************************************/

				/* IMPORTANT! Go some levels "down" in this method!
				 *   LoRa_Sleep > RN2483_Sleep > RN2483_ProcessSleepCommand > Leuart_SendCommand (added code in this method) */

#ifdef SKIPLORA
				LoRa_Sleep(A_MINUTE, &wakeUp);
#else
				LoRa_Sleep(5*A_MINUTE, &wakeUp);
#endif

				/* END ADDED CODE *******************************************************************************************/

				// wake up because of button pressed
				if(wakeUp){ // get out of bed early
					appState = WAKE_UP;
				}
				else{ // sleep time has passed
					appState = MEASURE;
				}
			} break;
			case DEEP_SLEEP:{
				/* BEGIN ADDED CODE *****************************************************************************************/
				dbprintln("> DEEP SLEEP");
				/* END ADDED CODE *******************************************************************************************/

				// save lora settings
				Lis3dh_DisableInterruptPin();
				//System_DeepSleep(SENS_EXT_ON);
				System_DeepSleep(NONE_ON);
			} break;
			case WAKE_UP:{
				/* BEGIN ADDED CODE *****************************************************************************************/
				dbprintln("> WAKE UP");
				/* END ADDED CODE *******************************************************************************************/

				LoRa_WakeUp();
				LED_Off();
				appState = DEEP_SLEEP;
			} break;
			default:{
				LED_HALTED();
			} break;
		}
	}
}
