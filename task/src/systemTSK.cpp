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
#include "timegm.h"

#define PIECE_BUF_RX		UART2_RxBffSz
#define connectUart			uart2

/*!****************************************************************************
 * @brief
 */
void systemTSK(void *pPrm){
	(void)pPrm;

	static SemaphoreHandle_t connUartTxSem;
	static SemaphoreHandle_t connUartRxSem;
	vSemaphoreCreateBinary(connUartTxSem);
	xSemaphoreTake(connUartTxSem, portMAX_DELAY);
	assert(connUartTxSem != NULL);
	vSemaphoreCreateBinary(connUartRxSem);
	xSemaphoreTake(connUartRxSem, portMAX_DELAY);
	assert(connUartRxSem != NULL);

	assert(pdTRUE == xTaskCreate(adcTSK, "adcTSK", ADC_TSK_SZ_STACK, NULL, ADC_TSK_PRIO, NULL));
	assert(pdTRUE == xTaskCreate(ds18TSK, "ds18TSK", DS18B_TSK_SZ_STACK, NULL, DS18B_TSK_PRIO, NULL));

	{	// Set timezone and DST
		char str[64];
		snprintf(str, sizeof(str), "TZ=CEST-2CET-3,M3.5.0/03:00:00,M10.5.0/04:00:00"); // TODO how this configure?
		//											| month 3, last Sunday, at 3:00am
		//															| month 10, last Sunday at 4:00AM
		putenv(str);
		tzset();
	}

	// uart RX TX callback
	auto uartRxHook = [](uart_type *puart){
		(void)puart;
		BaseType_t xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(connUartRxSem, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	};
	auto uartTxHook = [](uart_type *puart){
		(void)puart;
		BaseType_t xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(connUartTxSem, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	};

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
						if(minmea_parse_rmc(&frame, line) && frame.valid){
							struct tm gpstm = {};
							gpstm.tm_year = frame.date.year + 100;
							gpstm.tm_mon = frame.date.month - 1;
							gpstm.tm_mday = frame.date.day;
							gpstm.tm_hour = frame.time.hours;
							gpstm.tm_min = frame.time.minutes;
							gpstm.tm_sec = frame.time.seconds;
							gpstm.tm_isdst = 0;
							time_t gpsUnixTime = timegm(&gpstm);

							struct tm tm;
							localtime_r(&gpsUnixTime, &tm);
							char s[32];
							snprintf(s, sizeof(s), "%02i:%02i:%02i", tm.tm_hour, tm.tm_min, tm.tm_sec);
							display.putstring(0, 0, s);
							snprintf(s, sizeof(s), "%02i:%02i:%02i", tm.tm_mday, tm.tm_mon + 1, tm.tm_year - 100);
							display.putstring(0, 1, s);
						}else{
							display.putstring(0, 0, "--:--:--");
							display.putstring(0, 1, "--.--.--");
						}
					} break;
					default: ;
				}
				line = strtok_r(NULL, separator, &lasts);
			}
		}

		// Show outer temperature
		char s[16];
		if(temperature.state == temp_Ok){
			snprintf(s, sizeof(s), "%+" PRIi16 ".%" PRIu16 "\x7D", temperature.temperature / 10, abs(temperature.temperature) % 10);
		}else{
			snprintf(s, sizeof(s), "---\x7D");
		}
		display.putstring(10, 0, s);

		snprintf(s, sizeof(s), "\x7E" "%03u", adcTaskStct.filtered.lightSensorValue);
		display.putstring(10, 1, s);

		// Set display brightness
		uint8_t displayBrightness = 4;
		if(adcTaskStct.filtered.lightSensorValue < 100){
			displayBrightness = 4;
		}else if(adcTaskStct.filtered.lightSensorValue < 300){
			displayBrightness = 3;
		}else if(adcTaskStct.filtered.lightSensorValue < 2000){
			displayBrightness = 2;
		}else{
			displayBrightness = 1;
		}
		display.brightness(displayBrightness);

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
