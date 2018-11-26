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
 *         File: leuart.c
 *      Created: 2018-01-26
 *       Author: Guus Leenders
 *
 *  Description: This file contains the LEUART (serial communication)
 *  	functionality required by the RN2483 LoRa modem.
 *
 * Code added by Brecht Van Eeckhoudt.
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include <em_device.h>
#include <em_chip.h>
#include <em_gpio.h>
#include <em_emu.h>
#include <em_leuart.h>
#include <em_dma.h>
#include <em_cmu.h>

#include <dmactrl.h>
#include <rtcdriver.h>

#include "delay.h"
#include "util.h"
#include "leuart.h"
#include "pin_mapping.h"

/* BEGIN ADDED CODE *****************************************************************************************/
#include "dbprint.h" /* Homebrew printf */
#include "../src/main.h"
/* END ADDED CODE *******************************************************************************************/

#define TX_TIMEOUT_DURATION		2000	// ms
#define RX_TIMEOUT_DURATION		10000	// ms

/** DMA Configurations            */
#define DMA_CHANNEL_TX       0          /* DMA channel is 0 */
#define DMA_CHANNEL_RX       1
#define DMA_CHANNELS 		 2

/** DMA callback structure */
static DMA_CB_TypeDef dmaCallBack[DMA_CHANNELS];

//char commandBuffer[COMMAND_BUFFER_SIZE];
char receiveBuffer[RECEIVE_BUFFER_SIZE];
volatile uint8_t bufferPointer = 0;
volatile bool receiveComplete = false;

RTCDRV_TimerID_t xTimerForTimeout;
static volatile bool timeout = false;

void Leuart_ClearBuffers(void){
	memset(receiveBuffer, '\0', RECEIVE_BUFFER_SIZE);
	receiveComplete = false;
}

void timeoutCb(RTCDRV_TimerID_t id, void *user){
	timeout = true;
}

/** Static (internal) functions */
static void basicTxComplete(unsigned int channel, bool primary, void *user){
	(void) user;
	/* Refresh DMA basic transaction cycle */
	DMA_ActivateBasic(DMA_CHANNEL_RX,
					primary,
					false,
					(void *)&receiveBuffer[0],
					(void *)&RN2483_UART->RXDATA,
					0);
	bufferPointer = 0;
}

static void basicRxComplete(unsigned int channel, bool primary, void *user){
	(void) user;

	/* Refresh DMA basic transaction cycle */
	char c = RN2483_UART->RXDATA;
	if(bufferPointer < RECEIVE_BUFFER_SIZE - 1){
		if(c != '\n'){
			bufferPointer++;
			DMA_ActivateBasic(DMA_CHANNEL_RX,
								  primary,
								  false,
								  &receiveBuffer[bufferPointer],
								  NULL,
								  0);
		}
		else{
			receiveComplete = true;
			receiveBuffer[bufferPointer+1] = '\0';
		}
	}
}

static bool Leuart_ResponseAvailable(void){
	return receiveComplete;
}

void setupDma(void){
	/* DMA configuration structs */
	DMA_Init_TypeDef       dmaInit;
	DMA_CfgChannel_TypeDef rxChnlCfg;
	DMA_CfgChannel_TypeDef txChnlCfg;
	DMA_CfgDescr_TypeDef   rxDescrCfg;
	DMA_CfgDescr_TypeDef   txDescrCfg;

	/* Initializing the DMA */
	dmaInit.hprot        = 0;
	dmaInit.controlBlock = dmaControlBlock;
	DMA_Init(&dmaInit);

	/* RX DMA setup*/
	/* Set the interrupt callback routine */
	dmaCallBack[DMA_CHANNEL_RX].cbFunc = basicRxComplete;
	/* Callback doesn't need userpointer */
	dmaCallBack[DMA_CHANNEL_RX].userPtr = NULL;

	/* Setting up channel for TX*/
	rxChnlCfg.highPri   = false; /* Can't use with peripherals */
	rxChnlCfg.enableInt = true;  /* Enabling interrupt to refresh DMA cycle*/
	/*Setting up DMA transfer trigger request*/
	rxChnlCfg.select = DMAREQ_LEUART0_RXDATAV; //DMAREQ_LEUART0_RXDATAV;
	/* Setting up callback function to refresh descriptors*/
	rxChnlCfg.cb     = &(dmaCallBack[DMA_CHANNEL_RX]);
	DMA_CfgChannel(DMA_CHANNEL_RX, &rxChnlCfg);

	/* Setting up channel descriptor */
	/* Destination is buffer, increment ourselves */
	rxDescrCfg.dstInc = dmaDataIncNone;
	/* Source is LEUART_RX register and transfers 8 bits each time */
	rxDescrCfg.srcInc = dmaDataIncNone;
	rxDescrCfg.size   = dmaDataSize1;
	/* Default setting of DMA arbitration*/
	rxDescrCfg.arbRate = dmaArbitrate1;
	rxDescrCfg.hprot   = 0;
	/* Configure primary descriptor */
	DMA_CfgDescr(DMA_CHANNEL_RX, true, &rxDescrCfg);
	DMA_CfgDescr(DMA_CHANNEL_RX, false, &rxDescrCfg);

	/* TX DMA setup*/
	/* Set the interrupt callback routine */
	dmaCallBack[DMA_CHANNEL_TX].cbFunc = basicTxComplete;
	/* Callback doesn't need userpointer */
	dmaCallBack[DMA_CHANNEL_TX].userPtr = NULL;

	/* Setting up channel for TX*/
	txChnlCfg.highPri   = false; /* Can't use with peripherals */
	txChnlCfg.enableInt = true;  /* Enabling interrupt to refresh DMA cycle*/
	/*Setting up DMA transfer trigger request*/
	txChnlCfg.select = DMAREQ_LEUART0_TXBL; //DMAREQ_LEUART0_RXDATAV;
	/* Setting up callback function to refresh descriptors*/
	txChnlCfg.cb     = &(dmaCallBack[DMA_CHANNEL_TX]);
	DMA_CfgChannel(DMA_CHANNEL_TX, &txChnlCfg);

	/* Setting up channel descriptor */
	/* Destination is LEUART_Tx register and doesn't move */
	txDescrCfg.dstInc = dmaDataIncNone;
	/* Source is LEUART_TX register and transfers 8 bits each time */
	txDescrCfg.srcInc = dmaDataInc1;
	txDescrCfg.size   = dmaDataSize1;
	/* Default setting of DMA arbitration*/
	txDescrCfg.arbRate = dmaArbitrate1;
	txDescrCfg.hprot   = 0;
	/* Configure primary descriptor */
	DMA_CfgDescr(DMA_CHANNEL_TX, true, &txDescrCfg);
}

static void sendLeuartData(char * buffer, uint8_t bufferLength){
	// Wait for sync
	while (RN2483_UART->SYNCBUSY);

	DMA_ActivateBasic(DMA_CHANNEL_TX,
	                  true,
	                  false,
	                  (void *)&RN2483_UART->TXDATA,
	                  buffer,
	                  (unsigned int)(bufferLength - 1));

	 while(DMA_ChannelEnabled(DMA_CHANNEL_TX)); // EnterEM
}

static void setupLeuart(void){
	/* Allocate timer for Timeout detection*/
	RTCDRV_AllocateTimer(&xTimerForTimeout);

	/* Enable peripheral clocks */
	CMU_ClockEnable(cmuClock_HFPER, true);
	/* Configure GPIO pins */
	CMU_ClockEnable(cmuClock_GPIO, true);
	/* To avoid false start, configure output as high */
	GPIO_PinModeSet(RN2483_TX_PORT, RN2483_TX_PIN, gpioModePushPull, 1);
	GPIO_PinModeSet(RN2483_RX_PORT, RN2483_RX_PIN, gpioModeInput, 0);

	LEUART_Init_TypeDef init = LEUART_INIT_DEFAULT; // Default config is fine
	init.baudrate = 4800;

	/* Enable CORE LE clock in order to access LE modules */
	CMU_ClockEnable(cmuClock_CORELE, true);

	/* Select LFRCO for LEUARTs (and wait for it to stabilize) */
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
	CMU_ClockEnable(cmuClock_LEUART0, true);

	/* Do not prescale clock */
	CMU_ClockDivSet(cmuClock_LEUART0, cmuClkDiv_1);

	/* Configure LEUART */
	init.enable = leuartDisable;

	LEUART_Init(RN2483_UART, &init);

	/* Enable pins at default location */
	RN2483_UART->ROUTE = LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN | RN2483_UART_LOC;

	/* Set RXDMAWU to wake up the DMA controller in EM2 */
	LEUART_RxDmaInEM2Enable(RN2483_UART, true);

	/* Clear previous RX interrupts */
	LEUART_IntClear(RN2483_UART, LEUART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(LEUART0_IRQn);

	/* Finally enable it */
	LEUART_Enable(RN2483_UART, leuartEnable);
}

void Leuart_Init(void){
	setupDma();
	Leuart_BreakCondition();
	setupLeuart();

	// Auto baud setting
	char b[] = "U";
	sendLeuartData(b, 1);
	DelayMs(500);
}

void Leuart_Reinit(void){
	LEUART_Reset(RN2483_UART);
	Leuart_BreakCondition();
	setupLeuart();

	// Auto baud setting
	char b[] = "U";
	sendLeuartData(b, 1);
	Leuart_WaitForResponse();
	DelayMs(20);
	Leuart_WaitForResponse();
}

void Leuart_BreakCondition(void){
	GPIO_PinModeSet(RN2483_TX_PORT, RN2483_TX_PIN, gpioModePushPull, 1);
	DelayMs(40);
	GPIO_PinModeSet(RN2483_TX_PORT, RN2483_TX_PIN, gpioModePushPull, 0);
	DelayMs(20);
	GPIO_PinOutSet(RN2483_TX_PORT, RN2483_TX_PIN);
}

void Leuart_ReadResponse(char * buffer, uint8_t bufferLength){
	sprintf(buffer, "%s", receiveBuffer);
	receiveComplete = false;
	bufferPointer = 0;
}

void Leuart_SendData(char * buffer, uint8_t bufferLength){
	// Send data over LEUART
	sendLeuartData(buffer, bufferLength);

	// Start Timeout-timer if needed
	timeout = false;
	RTCDRV_StartTimer(xTimerForTimeout, rtcdrvTimerTypeOneshot, 10, (RTCDRV_Callback_t) &timeoutCb, NULL);
	// Wait for response or timeout
	while(!Leuart_ResponseAvailable() && !timeout){
		EMU_EnterEM2(true);
	}

	receiveComplete = true;
}

// Send a command string over the LEUART. Specifying a waitTime > 0 will result in a timeout when no command is received in time.
Leuart_Status_t Leuart_SendCommand(char * cb, uint8_t cbl, volatile bool * wakeUp){
	// Send data over LEUART
	sendLeuartData(cb, cbl);

	// Start Timeout-timer if needed
	timeout = false;
	//RTCDRV_StartTimer(xTimerForTimeout, rtcdrvTimerTypeOneshot, waitTime, (RTCDRV_Callback_t) &timeoutCb, NULL);
	// Wait for response or timeout

	/* BEGIN ADDED CODE *****************************************************************************************/
	if (appState == SLEEP)
	{
		dbprintln("INFO: Normally \"EMU_EnterEM2\" would be called here but MCU stays awake.");
		dbprintln(">> Functionality to receive and parse strings is now active.");
	}

	while(!Leuart_ResponseAvailable() && !timeout && !(*wakeUp)){
		//EMU_EnterEM2(true);

		/* BEGIN ADDED CODE (interrupt handling) *****************************************************************************************/

		/* Data is ready (notified by the RX handler) */
		if (dbprint_rxdata)
		{
			/* Disable "RX Data Valid Interrupt Enable" and "TX Complete Interrupt Enable" interrupts */
			USART_IntDisable(dbpointer, USART_IEN_RXDATAV);
			USART_IntDisable(dbpointer, USART_IEN_TXC);


			/* Copy received data in new buffers to be parsed later */
			if (!receiveBuffer0_used || !receiveBuffer1_used || !receiveBuffer2_used)
			{
				/* Copy data from the RX buffer to another buffer */
				for (uint16_t i = 0; dbprint_rx_buffer[i] != 0 && i < DBPRINT_BUFFER_SIZE-3; i++)
				{
					if (!receiveBuffer0_used) receiveBuffer0[i] = dbprint_rx_buffer[i];
					else if (!receiveBuffer1_used) receiveBuffer1[i] = dbprint_rx_buffer[i];
					else if (!receiveBuffer2_used) receiveBuffer2[i] = dbprint_rx_buffer[i];
				}

				/* Mark buffers as used */
				if (!receiveBuffer0_used)
				{
					receiveBuffer0_used = true;
					dbprintln("INFO: receiveBuffer0_used");
					if (data0_ready && data1_ready && receiveBuffer0_used && receiveBuffer1_used && receiveBuffer2_used)
					{
						dbprintln("WARN: All of the data is ready and all of the buffers are filled!");
					}
				}
				else if (!receiveBuffer1_used)
				{
					receiveBuffer1_used = true;
					dbprintln("INFO: receiveBuffer1_used");
					if (data0_ready && data1_ready && receiveBuffer0_used && receiveBuffer1_used && receiveBuffer2_used)
					{
						dbprintln("WARN: All of the data is ready and all of the buffers are filled!");
					}
				}
				else if (!receiveBuffer2_used)
				{
					receiveBuffer2_used = true;
					dbprintln("INFO: receiveBuffer2_used");
					if (data0_ready && data1_ready && receiveBuffer0_used && receiveBuffer1_used && receiveBuffer2_used)
					{
						dbprintln("WARN: All of the data is ready and all of the buffers are filled!");
					}
				}
			}
			else
			{
				dbprintln("CRIT: All buffers full but still new data!!");
				LED_ERROR(9);
			}

			/* Reset "notification" variable */
			dbprint_rxdata = false;

			/* Enable "RX Data Valid Interrupt" and "TX Complete Interrupt" interrupts */
			USART_IntEnable(dbpointer, USART_IEN_RXDATAV);
			USART_IntEnable(dbpointer, USART_IEN_TXC);
		}

		/* END ADDED CODE (interrupt handling) *******************************************************************************************/


		/* BEGIN ADDED CODE (parsing) *****************************************************************************************/

		/* Parse data if one of the buffers is used AND not all of the data-fields are filled */
		if ((receiveBuffer0_used || receiveBuffer1_used || receiveBuffer2_used) && !(data0_ready && data1_ready))
		{
			char idBuf[4]; /* ID is by design 3 characters + NULL termination */
			idBuf[3] = '\0'; /* NULL termination */
			char rssiBuf[3]; /* RSSI is by design 2 characters + NULL termination */
			rssiBuf[2] = '\0'; /* NULL termination */
			char vbatBuf[4]; /* VBAT is by design3 characters + NULL termination */
			vbatBuf[3] = '\0'; /* NULL termination */

			/* Loop through the string and separate fields */
			for (uint8_t i = 0; i < 8; i++)
			{
				/* First 3 characters = ID */
				if (i < 3)
				{
					if (receiveBuffer0_used) idBuf[i] = receiveBuffer0[i];
					else if (receiveBuffer1_used) idBuf[i] = receiveBuffer1[i];
					else if (receiveBuffer2_used) idBuf[i] = receiveBuffer2[i];
				}
				/* Next 2 characters = RSSI */
				else if ((i > 2) && (i < 5))
				{
					if (receiveBuffer0_used) rssiBuf[i-3] = receiveBuffer0[i];
					else if (receiveBuffer1_used) rssiBuf[i-3] = receiveBuffer1[i];
					else if (receiveBuffer2_used) rssiBuf[i-3] = receiveBuffer2[i];
				}
				/* Next (last) 3 characters = VBAT */
				else if ((i > 4) && (i < 8))
				{
					if (receiveBuffer0_used) vbatBuf[i-5] = receiveBuffer0[i];
					else if (receiveBuffer1_used) vbatBuf[i-5] = receiveBuffer1[i];
					else if (receiveBuffer2_used) vbatBuf[i-5] = receiveBuffer2[i];
				}
			}

			/* Parse the separated fields to values, mark the filled data-fields to
			 * "ready" and mark the buffer as "empty" */
			if (!data0_ready)
			{
				/* Casting just in case */
				id0 = (uint8_t)(charDec_to_uint32(idBuf));
				rssi0 = (uint8_t)(charDec_to_uint32(rssiBuf));
				vbat0 = (uint16_t)(charDec_to_uint32(vbatBuf));
				data0_ready = true;
				dbprint("INFO: data0_ready - ");

				if (receiveBuffer0_used)
				{
					receiveBuffer0_used = false;
					dbprintln("receiveBuffer0 is free");
				}
				else if (receiveBuffer1_used)
				{
					receiveBuffer1_used = false;
					dbprintln("receiveBuffer1 is free");
				}
				else if (receiveBuffer2_used)
				{
					receiveBuffer2_used = false;
					dbprintln("receiveBuffer2 is free");
				}
			}
			else if (!data1_ready)
			{
				/* Casting just in case */
				id1 = (uint8_t)(charDec_to_uint32(idBuf));
				rssi1 = (uint8_t)(charDec_to_uint32(rssiBuf));
				vbat1 = (uint16_t)(charDec_to_uint32(vbatBuf));
				data1_ready = true;
				dbprint("INFO: data1_ready - ");

				if (receiveBuffer0_used)
				{
					receiveBuffer0_used = false;
					dbprintln("receiveBuffer0 is free");
				}
				else if (receiveBuffer1_used)
				{
					receiveBuffer1_used = false;
					dbprintln("receiveBuffer1 is free");
				}
				else if (receiveBuffer2_used)
				{
					receiveBuffer2_used = false;
					dbprintln("receiveBuffer2 is free");
				}
			}
			else
			{
				dbprintln("CRIT: Tried to store new data, no more space!");
				LED_ERROR(10);
			}
		}

		/* END ADDED CODE (parsing) *******************************************************************************************/
	}
	/* END ADDED CODE *******************************************************************************************/

	// Check for timeout
	if(timeout){ // if running is false, we got here because of the timeout.
		return TX_TIMEOUT;
	}

	// Stop Timeout-timer when command is received
	//RTCDRV_StopTimer(xTimerForTimeout); // stop timer

	return DATA_SENT;
}


Leuart_Status_t Leuart_WaitForResponse(){
	// Activate DMA
	DMA_ActivateBasic(	DMA_CHANNEL_RX,
						true,
						false,
						(void *)&receiveBuffer[0],
						(void *)&RN2483_UART->RXDATA,
						0);

	// Start Timeout-timer
	timeout = false;
	//RTCDRV_StartTimer(xTimerForTimeout, rtcdrvTimerTypeOneshot, RX_TIMEOUT_DURATION, (RTCDRV_Callback_t) &timeoutCb, NULL);
	// Wait for response or timeout
	while(!Leuart_ResponseAvailable() && !timeout){
		EMU_EnterEM2(true);
	}
	//RTCDRV_StopTimer(xTimerForTimeout); // stop timer

	// Check for timeout
	if(timeout){ // if running is false, we got here because of the timeout.
		return RX_TIMEOUT;
	}

	// OK
	return DATA_RECEIVED;
}
