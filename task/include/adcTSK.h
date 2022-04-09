/*!****************************************************************************
 * @file    	adcTSK.h
 * @author  	Storozhenko Roman - D_EL
 * @version 	V1.0
 * @date    	17-01-2022
 * @copyright 	The MIT License (MIT). Copyright (c) 2022 Storozhenko Roman
 */
#ifndef ADC_TSK_H
#define ADC_TSK_H

/*!****************************************************************************
 * Include
 */

/*!****************************************************************************
 * User define
 */
#define MA_FILTER_MAX_WITH			(64)

/*!****************************************************************************
 * User typedef
 */
typedef struct {
	uint16_t adcDefVal;
	uint16_t oversampling;
	uint16_t recursiveK;
	uint16_t MA_filter_WITH;

	uint32_t recursiveFilterCumul;
	uint16_t recursiveFilterOut;
	uint16_t MA_filterMas[MA_FILTER_MAX_WITH];
	uint16_t MA_filterIndex;
	uint32_t MA_accumulator;
} adcFilt_type;

typedef struct {
	adcFilt_type	adcFilt[CH_NUMBER];
	uint16_t		filtered[CH_NUMBER];
	uint16_t		targetcurrent;
} adcTaskStct_type;

/*!****************************************************************************
 * External variables
 */
extern adcTaskStct_type adcTaskStct;

/*!****************************************************************************
 * Prototypes for the functions
 */
void adcTSK(void *pPrm);

#endif //ADC_TSK_H
/******************************** END OF FILE ********************************/
