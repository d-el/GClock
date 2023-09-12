/*!****************************************************************************
 * @file		mh-z19.h
 * @author		d_el
 * @version		V1.0
 * @date		01.11.2022
 * @copyright	License (MIT). Copyright (c) 2022 Storozhenko Roman
 * @brief
 */

#ifndef mh_z19_H
#define mh_z19_H

/*!****************************************************************************
 * Include
 */
#include <stdint.h>


/*!****************************************************************************
 * Function declaration
 */
void mh_z19_init(void);
int16_t mh_z19_readCO2(void);
bool mh_z19_zeroPointCalibration();
bool mh_z19_spanPointCalibration(uint16_t span);
bool mh_z19_abcLogicOn(bool on);
bool mh_z19_sensorRange(uint16_t range); // Detection range is 2000 or 5000ppm

#endif //mh_z19_H
/******************************** END OF FILE ********************************/
