/*!****************************************************************************
 * @file    	pwm.h
 * @author  	Storozhenko Roman - D_EL
 * @version 	V1.0
 * @date    	02.05.2016
 * @copyright 	The MIT License (MIT). Copyright (c) 2020 Storozhenko Roman
 */
#ifndef pwm_H
#define pwm_H

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
#define PWM_FREQ    40000   //[Hz]

/*!****************************************************************************
* Prototypes for the functions
*/
void pwm_init(void);
void pwm1set(int32_t dc);
void pwm2set(int32_t dc);

#ifdef __cplusplus
}
#endif

#endif //pwm_H
/******************************** END OF FILE ********************************/
