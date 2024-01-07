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
#include <algorithm>
#include "ds18TSK.h"
#include <enco.h>
#include <base.h>
#include <prmSystem.h>
#include <plog.h>
#include <version.h>
#include <display.h>
#include "sensorTSK.h"
#include "time.h"

extern "C" int _write(int file, const void *ptr, unsigned int len);

#define LOG_LOCAL_LEVEL P_LOG_INFO
static const char *logTag = "systemTSK";

/*!****************************************************************************
 * @brief
 */

volatile uint16_t reg = 0xFFFF;
DMA_TypeDef *dmaisr = DMA1;

void systemTSK(void *pPrm){
	(void)pPrm;

	disp_init();
	disp_setColor(black, white);

	//Init log system
	plog_setVprintf(vsprintf);
	plog_setWrite(_write);
	plog_setTimestamp(xTaskGetTickCount);
	P_LOGI(logTag, "Version %s", getVersion());


	assert(pdTRUE == xTaskCreate(adcTSK, "adcTSK", ADC_TSK_SZ_STACK, NULL, ADC_TSK_PRIO, NULL));
	assert(pdTRUE == xTaskCreate(ds18TSK, "ds18TSK", DS18B_TSK_SZ_STACK, NULL, DS18B_TSK_PRIO, NULL));
	assert(pdTRUE == xTaskCreate(sensorTSK, "sensorTSK", SENSOR_TSK_SZ_STACK, NULL, SENSOR_TSK_PRIO, NULL));

	{	// Set timezone and DST
		char str[64];
		snprintf(str, sizeof(str), "TZ=CEST-2CET-3,M3.5.0/03:00:00,M10.5.0/04:00:00"); // TODO how this configure?
		//											| month 3, last Sunday, at 3:00am
		//															| month 10, last Sunday at 4:00AM
		putenv(str);
		tzset();
	}

	enco_init();

	//vTaskDelay(pdMS_TO_TICKS(500));
	assert(pdTRUE == xTaskCreate(baseTSK, "baseTSK", WINDOW_TSK_SZ_STACK, NULL, WINDOW_TSK_PRIO, NULL));

	while(1){
		Prm::gtime.val = time(NULL);

		Prm::temp_out_ok.val = ds18b20data[1].state == temp_ok ? 1 : 0;
		Prm::temp_out.val = ds18b20data[1].temperature;
		if(ds18b20data[1].state == temp_ok && Prm::gtime.val){
			if(Prm::temp_out.val > Prm::temp_out_max.val){
				Prm::temp_out_max.val = Prm::temp_out.val;
				Prm::temp_out_max_time.val = Prm::gtime.val;
			}
			if(Prm::temp_out.val < Prm::temp_out_min.val){
				Prm::temp_out_min.val = Prm::temp_out.val;
				Prm::temp_out_min_time.val = Prm::gtime.val;
			}
		}

		Prm::temp_in_ok.val = ds18b20data[0].state == temp_ok ? 1 : 0;
		Prm::temp_in.val = ds18b20data[0].temperature;
		if(ds18b20data[0].state == temp_ok && Prm::gtime.val){
			if(Prm::temp_in.val > Prm::temp_in_max.val){
				Prm::temp_in_max.val = Prm::temp_in.val;
				Prm::temp_in_max_time.val = Prm::gtime.val;
			}
			if(Prm::temp_in.val < Prm::temp_in_min.val){
				Prm::temp_in_min.val = Prm::temp_in.val;
				Prm::temp_in_min_time.val = Prm::gtime.val;
			}
		}

		Prm::light = adcTaskStct.filtered.lightSensorValue;

		vTaskDelay(100);
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
