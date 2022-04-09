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
#include <arm_math.h>
#include <pwm.h>
#include <gpio.h>
#include "adcTSK.h"

/*!****************************************************************************
 * Local function declaration
 */
static inline uint16_t movingAverageFilter(adcFilt_type *f, uint16_t v);
static inline void aInit(void);
static void adcHoock(adcStct_type *adc);

/*!****************************************************************************
 * MEMORY
 */
adcTaskStct_type adcTaskStct = {
	.adcFilt = {
		[CH_UINADC] =		{ .adcDefVal = 0, .oversampling = 1, .recursiveK = 1, .MA_filter_WITH = 64 },
		[CH_ILED1] =		{ .adcDefVal = 0, .oversampling = 8, .recursiveK = 1, .MA_filter_WITH = 64 },
		[CH_ILED2] =		{ .adcDefVal = 0, .oversampling = 8, .recursiveK = 1, .MA_filter_WITH = 64 },
		[CH_TEMPERATURE] =	{ .adcDefVal = 0, .oversampling = 1, .recursiveK = 1, .MA_filter_WITH = 64 },
		[CH_VREF] =			{ .adcDefVal = 0, .oversampling = 1, .recursiveK = 1, .MA_filter_WITH = 64 }
	}
};
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
	aInit();
	adc_setSampleRate(100);
	adc_init();
	adc_startSampling();

	while(1){
		xSemaphoreTake(AdcEndConversionSem, portMAX_DELAY);

		if(a.targetcurrent > 0){
			for(uint8_t index = 0; index < CH_NUMBER; index++){
				// Oversampling
				uint32_t val = adcValue.adcreg[index] * a.adcFilt[index].oversampling;
				// Apply filter
				a.filtered[index] = movingAverageFilter(&a.adcFilt[index], val);
			}
		}
	}
}

/*!****************************************************************************
 * @brief	Init to default adc task memory
 */
static inline void aInit(void){
	for(uint8_t ch = 0; ch < CH_NUMBER; ch++){
		adcTaskStct.adcFilt[ch].recursiveFilterCumul =
				adcTaskStct.adcFilt[ch].adcDefVal << adcTaskStct.adcFilt[ch].recursiveK;
		for(uint16_t i = 0; i < adcTaskStct.adcFilt[ch].MA_filter_WITH; i++){
			adcTaskStct.adcFilt[ch].MA_filterMas[i] = adcTaskStct.adcFilt[ch].adcDefVal;
		}
	}
}

/*!****************************************************************************
 * @brief
 */
static inline uint16_t movingAverageFilter(adcFilt_type *f, uint16_t v){
	f->MA_accumulator -= f->MA_filterMas[f->MA_filterIndex];
	f->MA_filterMas[f->MA_filterIndex] = v;
	f->MA_accumulator += f->MA_filterMas[f->MA_filterIndex];

	f->MA_filterIndex++;
	if(f->MA_filterIndex >= f->MA_filter_WITH){
		f->MA_filterIndex = 0;
	}

	return f->MA_accumulator / f->MA_filter_WITH;
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
