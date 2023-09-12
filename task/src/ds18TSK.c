/*!****************************************************************************
 * @file		ds18TSK.c
 * @author		d_el
 * @version		V1.1
 * @date		06.04.2022
 * @brief
 * @copyright	The MIT License (MIT). Copyright (c) 2022 Storozhenko Roman
 */

/*!****************************************************************************
* Include
*/
#include <string.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <ds18b20.h>
#include <oneWireUart.h>
#include <crc.h>
#include <uart.h>
#include "ds18TSK.h"

/*!****************************************************************************
* MEMORY
*/
ds18b20data_type ds18b20data[DS18_MAX_SENSORS];

/*!****************************************************************************
* @brief
* @param
* @retval
*/
void ds18TSK(void *pPrm){
	(void)pPrm;
	uint8_t errorcnt = 0;

	for(uint8_t i = 0; i < DS18_MAX_SENSORS; i++){
		ds18b20data[i].state = temp_noInit;
	}

	ow_init();
	vTaskDelay(30);	// For stabilize line voltage and boot device

	ow_searchRomContext_t searchRomContext = {};
	for(uint8_t i = 0; i < DS18_MAX_SENSORS; i++){
		owSt_type res = ow_searchRom(&searchRomContext);
		if(res == owSearchOk || res == owSearchLast){
			memcpy(ds18b20data[i].rom, searchRomContext.rom, sizeof(ds18b20data[i].rom));
			ds18b20data[i].state = temp_presence;
		}
		if(res == owSearchLast){
			break;
		}
	}

	while(1){
		for(uint8_t i = 0; i < DS18_MAX_SENSORS; i++){
			// DS18B20 INIT
			while(ds18b20data[i].state == temp_presence || ds18b20data[i].state == temp_errSensor){
				ds18b20state_type resInit = ds18b20Init(ds18b20data[i].rom);
				if(resInit == ds18b20st_ok){
					ds18b20data[i].state = temp_init;
					ds18b20data[i].errorcnt = 0;
					break;
				}
				else{
					if(errorcnt > DS18_MAX_ERROR){
						ds18b20data[i].errorcnt++;
					}else{
						ds18b20data[i].state = temp_errSensor;
					}
				}
			}
		}

		ds18b20ConvertTemp(NULL); // Send to all sensors
		vTaskDelay(pdMS_TO_TICKS(1000));

		for(uint8_t i = 0; i < DS18_MAX_SENSORS; i++){
			if(ds18b20data[i].state == temp_init || ds18b20data[i].state == temp_ok){
				uint8_t scratchpad[9];
				ds18b20state_type res = ds18b20ReadScratchpad(ds18b20data[i].rom, scratchpad);
				if(res != ds18b20st_ok)
					goto error;
				ds18b20data[i].temperature = ds18b20Reg2tmpr(scratchpad[0], scratchpad[1]);
				ds18b20data[i].state = temp_ok;
				ds18b20data[i].errorcnt = 0;
			}
			continue;

			error:
				if(ds18b20data[i].errorcnt < DS18_MAX_ERROR){
					ds18b20data[i].errorcnt++;
				}else{
					ds18b20data[i].state = temp_errSensor;
				}
		}
	}
}

/******************************** END OF FILE ********************************/
