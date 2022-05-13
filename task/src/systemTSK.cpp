/******************************************************************************
 * @file		systemTSK.c
 * @author		Storozhenko Roman - D_EL
 * @version		V1.0
 * @date		14-09-2015
 * @copyright 	The MIT License (MIT). Copyright (c) 2020 Storozhenko Roman
 */

/*!****************************************************************************
* Include
*/
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <adc.h>
#include "systemTSK.h"
#include "adcTSK.h"
#include <movingAverageFilter.h>
#include <uart.h>
#include <minmea/minmea.h>
#include <algorithm>
#include "ds18TSK.h"
#include <datecs.h>

// MCU calibration data
#define CAL_VREF_DATA	(*(uint16_t*)0x1FFF75AA)	// ADC value VREF_INT at 3.0V VREF
#define CAL_TS_DATA		(*(uint16_t*)0x1FFF75A8)	// ADC value Temperature sensot at 30 °C 3.0V VREF
#define TS_LINEARITY	2.5							// mv / °C

static SemaphoreHandle_t connUartTxSem;
static SemaphoreHandle_t connUartRxSem;

#define PIECE_BUF_RX		UART2_RxBffSz
#define connectUart			uart2

/*!****************************************************************************
 * @brief	uart RX TX callback
 */
static void uartRxHook(uart_type *puart){
	(void)puart;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(connUartRxSem, &xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

static void uartTxHook(uart_type *puart){
	(void)puart;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(connUartTxSem, &xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

/*!****************************************************************************
 * @brief
 */
void systemTSK(void *pPrm){
	(void)pPrm;

	vSemaphoreCreateBinary(connUartTxSem);
	xSemaphoreTake(connUartTxSem, portMAX_DELAY);
	assert(connUartTxSem != NULL);
	vSemaphoreCreateBinary(connUartRxSem);
	xSemaphoreTake(connUartRxSem, portMAX_DELAY);
	assert(connUartRxSem != NULL);

	//assert(pdTRUE == xTaskCreate(adcTSK, "adcTSK", ADC_TSK_SZ_STACK, NULL, ADC_TSK_PRIO, NULL));
	assert(pdTRUE == xTaskCreate(ds18TSK, "ds18TSK", DS18B_TSK_SZ_STACK, NULL, DS18B_TSK_PRIO, NULL));

	uart_init(connectUart, 9600);
	uart_setCallback(connectUart, uartTxHook, uartRxHook);

	const uint32_t enableDelay = 2500;
	vTaskDelay(pdMS_TO_TICKS(enableDelay));

	auto dispalayInterface = [](const void* c, size_t len){
		uart_write(connectUart, c, len);
		xSemaphoreTake(connUartTxSem, pdMS_TO_TICKS(100));
	};
	Datecs display(dispalayInterface);
	display.init();

	// Startup animation
	for(size_t i = 0; i < 20; i++){
		display.clear();
		display.putstring(i, 0, ">");
		display.putstring(19 - i, 1, "<");
		display.flush();
		vTaskDelay(pdMS_TO_TICKS(50));
	}
	display.clear();
	display.putstring(6, 0, "DEL 2022");
	display.flush();
	vTaskDelay(pdMS_TO_TICKS(500));

	while(1){
		display.clear();

		// Read from GPS
		uart_read(connectUart, connectUart->pRxBff, PIECE_BUF_RX);
		BaseType_t res = xSemaphoreTake(connUartRxSem, pdMS_TO_TICKS(2000));
		size_t numRx = PIECE_BUF_RX - uartGetRemainRx(connectUart);

		// Parse GPS
		if((numRx != 0)&&(res == pdTRUE)){
			// Parse NMEA
			const char *separator = "\n";
			char *lasts = nullptr;
			char *line = strtok_r((char *)connectUart->pRxBff, separator, &lasts);
			while(line != nullptr && line[0] == '$'){
				switch(minmea_sentence_id(line, false)){
					case MINMEA_SENTENCE_GGA: {
						struct minmea_sentence_gga frame;
						if(minmea_parse_gga(&frame, line)){
							char s[4];
							snprintf(s, sizeof(s), "S%i", frame.satellites_tracked);
							display.putstring(17, 0, s);
						}
					} break;

					case MINMEA_SENTENCE_RMC: {
						struct minmea_sentence_rmc frame;
						if(minmea_parse_rmc(&frame, line)){
							struct tm gpstm = {};
							gpstm.tm_year = frame.date.year;
							gpstm.tm_mon = frame.date.month - 1;
							gpstm.tm_mday = frame.date.day;
							gpstm.tm_hour = frame.time.hours;
							gpstm.tm_min = frame.time.minutes;
							gpstm.tm_sec = frame.time.seconds;
							time_t gpstime = mktime(&gpstm) + 60 * 60 * 3; /* TODO */

							struct tm tm;
							localtime_r(&gpstime, &tm);
							if(gpstm.tm_year > 0){
								char s[16];
								snprintf(s, sizeof(s), "%02i:%02i:%02i", tm.tm_hour, tm.tm_min, tm.tm_sec);
								display.putstring(0, 0, s);
								snprintf(s, sizeof(s), "%02i:%02i:%02i", tm.tm_mday, tm.tm_mon, tm.tm_year);
								display.putstring(0, 1, s);
							}else{
								display.putstring(0, 0, "--:--:--");
								display.putstring(0, 1, "--.--.--");
							}
						}
					} break;
					default: ;
				}
				line = strtok_r(NULL, separator, &lasts);
			}
		}

		char s[16];
		if(temperature.state == temp_Ok){
			snprintf(s, sizeof(s), "%+" PRIi16 ".%" PRIu16 "\x7D", temperature.temperature / 10, abs(temperature.temperature) % 10);
		}else{
			snprintf(s, sizeof(s), "---\x7D");
		}
		display.putstring(10, 0, s);

		display.flush();
	}
}

/*!****************************************************************************
*
*/
void OSinit(void){
	BaseType_t res = xTaskCreate(systemTSK, "systemTSK", SYSTEM_TSK_SZ_STACK, NULL, SYSTEM_TSK_PRIO, NULL);
	assert(res == pdTRUE);
	vTaskStartScheduler();
}

/******************************** END OF FILE ********************************/
