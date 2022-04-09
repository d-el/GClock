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
temperature_type temperature;

/*!****************************************************************************
* @brief
* @param
* @retval
*/
void ds18TSK(void *pPrm){
	(void)pPrm;
	uint8_t errorcnt = 0;

	ow_init();
	temperature.state = temp_Ok;

	/*****************************
	* DS18B20 INIT
	*/
	while(1){
		ds18b20state_type resInit = ds18b20Init();
		if(resInit == ds18b20st_ok){
			temperature.state = temp_Ok;
			break;
		}
		else{
			if(errorcnt < DS18_MAX_ERROR){
				errorcnt++;
			}else{
				temperature.state = temp_NoInit;
			}
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}

	while(1){
		uint8_t bff[9];

		owSt_type st =  ow_reset();
		bff[0] = SKIP_ROM;
		ow_write(bff, 1);
		bff[0] = CONVERT_T;
		ow_write(bff, 1);

		ow_setOutHi();
		memset(bff, 0, 9);
		vTaskDelay(pdMS_TO_TICKS(1000));

		ow_setOutOpenDrain();
		st = ow_reset();
		if(st != owOk)
			goto error;

		bff[0] = SKIP_ROM;
		st = ow_write(bff, 1);
		if(st != owOk)
			goto error;

		bff[0] = READ_SCRATCHPAD;
		st = ow_write(bff, 1);
		if(st != owOk)
			goto error;

		st = ow_read(bff, 9);
		if(st != owOk)
			goto error;

		uint8_t crc = crc8Calc(&crc1Wire, bff, 9);
		if(crc != 0)
			goto error;

		int16_t scratchpad = bff[1];
		scratchpad <<= 8;
		scratchpad |= bff[0];
		temperature.temperature = (scratchpad * 10 + (16/2)) / 16; //Division with rounding
		temperature.state = temp_Ok;
		errorcnt = 0;

	continue;

	error:
			if(errorcnt < DS18_MAX_ERROR){
				errorcnt++;
			}else{
				temperature.state = temp_ErrSensor;
			}
	}
}

/******************************** END OF FILE ********************************/
