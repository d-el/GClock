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
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <adc.h>
#include "systemTSK.h"
#include "adcTSK.h"
#include <uart.h>
#include <minmea/minmea.h>
#include <algorithm>
#include "ds18TSK.h"
#include <datecs.h>
#include "timegm.h"
#include <mh-z19.h>
#include <enco.h>
#include <base.h>
#include <prmSystem.h>
#include <plog.h>
#include <version.h>

#define PIECE_BUF_RX		UART2_RxBffSz
#define connectUart			uart2

extern "C" int _write(int file, const void *ptr, unsigned int len);

#define LOG_LOCAL_LEVEL P_LOG_INFO
static const char *logTag = "systemTSK";

/*!****************************************************************************
 * @brief
 */
void systemTSK(void *pPrm){
	(void)pPrm;

	static SemaphoreHandle_t connUartTxSem;
	vSemaphoreCreateBinary(connUartTxSem);
	xSemaphoreTake(connUartTxSem, portMAX_DELAY);
	assert(connUartTxSem != NULL);

	static SemaphoreHandle_t connUartRxSem;
	vSemaphoreCreateBinary(connUartRxSem);
	xSemaphoreTake(connUartRxSem, portMAX_DELAY);
	assert(connUartRxSem != NULL);

//	static SemaphoreHandle_t displayUartMutex;
//	displayUartMutex = xSemaphoreCreateMutex();
//	assert(displayUartMutex != NULL);

	//Init log system
	plog_setVprintf(vsprintf);
	plog_setWrite(_write);
	plog_setTimestamp(xTaskGetTickCount);
	P_LOGI(logTag, "Version %s", getVersion());

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
		//xSemaphoreTake(displayUartMutex, portMAX_DELAY);
		uart_write(connectUart, c, len);
		xSemaphoreTake(connUartTxSem, pdMS_TO_TICKS(100));
		//xSemaphoreGive(displayUartMutex);
	};
	Datecs::get().setInterface(dispalayInterface);
	Datecs::get().init();

	mh_z19_init();
	enco_init();

	vTaskDelay(pdMS_TO_TICKS(500));
	assert(pdTRUE == xTaskCreate(baseTSK, "baseTSK", WINDOW_TSK_SZ_STACK, NULL, WINDOW_TSK_SZ_STACK, NULL));

	//xSemaphoreTake(displayUartMutex, portMAX_DELAY);
	uart_read(connectUart, connectUart->pRxBff, PIECE_BUF_RX);
	//xSemaphoreGive(displayUartMutex);
	while(1){
		// Read from GPS
		BaseType_t res = xSemaphoreTake(connUartRxSem, 0);
		size_t numRx = PIECE_BUF_RX - uartGetRemainRx(connectUart);

		// Parse GPS
		if((numRx != 0)&&(res == pdTRUE)){
			uart_read(connectUart, connectUart->pRxBff, PIECE_BUF_RX);

			// Parse NMEA
			const char *separator = "\n";
			char *lasts = nullptr;
			char *line = strtok_r((char *)connectUart->pRxBff, separator, &lasts);
			while(line != nullptr && line[0] == '$'){
				switch(minmea_sentence_id(line, false)){
					case MINMEA_SENTENCE_GGA: {
						struct minmea_sentence_gga frame;
						if(minmea_parse_gga(&frame, line)){
							Prm::satellites.val = frame.satellites_tracked;
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
							Prm::gtime.val = gpsUnixTime;
						}else{

						}
					} break;
					default: ;
				}
				line = strtok_r(NULL, separator, &lasts);
			}
		}

		Prm::co2.val = mh_z19_readCO2();
		if(Prm::co2setZero.val){
			mh_z19_abcLogicOn(true);
			mh_z19_zeroPointCalibration();
			Prm::co2setZero.val = 0;
			P_LOGI(logTag, "MH-Z19 send calibration zero point");
		}

		Prm::temp_out_ok.val = ds18b20data[1].state == temp_ok ? 1 : 0;
		Prm::temp_out.val = ds18b20data[1].temperature;
		if(ds18b20data[1].state == temp_ok && Prm::gtime.val){
			if(Prm::temp_out.val >= Prm::temp_out_max.val){
				Prm::temp_out_max.val = Prm::temp_out.val;
				Prm::temp_out_max_time.val = Prm::gtime.val;
			}
			if(Prm::temp_out.val <= Prm::temp_out_min.val){
				Prm::temp_out_min.val = Prm::temp_out.val;
				Prm::temp_out_min_time.val = Prm::gtime.val;
			}
		}

		Prm::temp_in_ok.val = ds18b20data[0].state == temp_ok ? 1 : 0;
		Prm::temp_in.val = ds18b20data[0].temperature;
		if(ds18b20data[0].state == temp_ok && Prm::gtime.val){
			if(Prm::temp_in.val >= Prm::temp_in_max.val){
				Prm::temp_in_max.val = Prm::temp_in.val;
				Prm::temp_in_max_time.val = Prm::gtime.val;
			}
			if(Prm::temp_in.val <= Prm::temp_in_min.val){
				Prm::temp_in_min.val = Prm::temp_in.val;
				Prm::temp_in_min_time.val = Prm::gtime.val;
			}
		}

		Prm::light = adcTaskStct.filtered.lightSensorValue;

		vTaskDelay(10);
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
