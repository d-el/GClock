/*!****************************************************************************
 * @file    	systemTSK.h
 * @author  	Storozhenko Roman - D_EL
 * @version 	V1.0
 * @date    	14-09-2015
 * @copyright 	The MIT License (MIT). Copyright (c) 2020 Storozhenko Roman
 */
#ifndef systemTSK_H
#define systemTSK_H

/*!****************************************************************************
* Include
*/

/*!****************************************************************************
* User define
*/
#define SYSTEM_TSK_PERIOD   (10)         ///<[ms]
#define CUR_OFF_TIME        (1000)      ///<[ms]
#define MAX_WAIT_RxRequest  (20)        ///<[ms]

#define COOLER_PWM_START    (0.3)       ///<[k PWM]
#define MIN_TEMP            (38.0)      ///<[°C]
#define MAX_TEMP            (60.0)      ///<[°C]
#define TEMP_OFF            (60.0)      ///<[°C]
#define H_TEMP              (5.0)       ///<[°C]

#define VTASK_FILTER_K      (3)

#define MIN_VIN_VOLTAGE		(35.0)		//[V]

/*!****************************************************************************
* Prototypes for the functions
*/
void OSinit(void);

#endif //systemTSK_H
/******************************** END OF FILE ********************************/
