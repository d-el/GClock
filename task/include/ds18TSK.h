/*!****************************************************************************
 * @file		ds18TSK.h
 * @author		d_el
 * @version		V1.1
 * @date		06.04.2022
 * @brief
 * @copyright	The MIT License (MIT). Copyright (c) 2022 Storozhenko Roman
 */
#ifndef ds18TSK_H
#define ds18TSK_H

#ifdef __cplusplus
extern "C" {
#endif

/*!****************************************************************************
* Include
*/
#include <stdint.h>

/*!****************************************************************************
* User define
*/
#define DS18_MAX_ERROR		(3)	//Maximum error before set error state
#define DS18_MAX_SENSORS	(3)
/*!****************************************************************************
* User enum
*/

/*!****************************************************************************
* User typedef
*/
typedef enum{
	temp_ok,
	temp_noInit,
	temp_presence,
	temp_init,
	temp_errSensor
}temperatureState_type;

typedef struct{
	uint8_t errorcnt;
	temperatureState_type state;
	int16_t temperature;			//[X_X °C]
	uint8_t rom[8];
}ds18b20data_type;

/*!****************************************************************************
* External variables
*/
extern ds18b20data_type ds18b20data[DS18_MAX_SENSORS];

/*!****************************************************************************
* Macro functions
*/

/*!****************************************************************************
* Prototypes for the functions
*/
void ds18TSK(void *pPrm);

#ifdef __cplusplus
}
#endif

#endif //ds18TSK_H
/******************************** END OF FILE ********************************/
