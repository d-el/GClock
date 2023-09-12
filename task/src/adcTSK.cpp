/*!****************************************************************************
 * @file		adcTSK.c
 * @author		Storozhenko Roman - D_EL
 * @version 	V1.0
 * @date		17-01-2022
 * @copyright 	The MIT License (MIT). Copyright (c) 2022 Storozhenko Roman
 */

/*!****************************************************************************
 * Include
 */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <adc.h>
#include <pwm.h>
#include <gpio.h>
#include "adcTSK.h"
#include <movingAverageFilter.h>

/*!****************************************************************************
 * Local function declaration
 */
static void adcHoock(adcStct_type *adc);

/*!****************************************************************************
 * MEMORY
 */
adcTaskStct_type adcTaskStct;
SemaphoreHandle_t AdcEndConversionSem;
adcStct_type adcValue;

/*!****************************************************************************
 * @brief
 * @param
 * @retval
 */
void adcTSK(void *pPrm){
	(void)pPrm;
	adcTaskStct_type& a = adcTaskStct;

	vSemaphoreCreateBinary(AdcEndConversionSem);
	xSemaphoreTake(AdcEndConversionSem, portMAX_DELAY);

	adc_setCallback(adcHoock);
	adc_setSampleRate(1000);
	adc_init();
	adc_startSampling();

	static MovingAverageFilter<uint16_t, 32> f_lightSensor(0);
	static MovingAverageFilter<uint16_t, 8> f_temperature(0);
	static MovingAverageFilter<uint16_t, 8> f_vref(0);

	while(1){
		xSemaphoreTake(AdcEndConversionSem, portMAX_DELAY);

		a.filtered.lightSensorValue = f_lightSensor.proc(adcValue.adcreg[CH_LIGHT_SENSOR]);
	}
}

/*!****************************************************************************
 * @brief
 */
static void adcHoock(adcStct_type *adc){
	adcValue = *adc;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(AdcEndConversionSem, &xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

/******************************** END OF FILE ********************************/
