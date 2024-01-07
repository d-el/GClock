/*!****************************************************************************
 * @file		hdc1080.c
 * @author		d_el
 * @version		V1.0
 * @date		Dec 18, 2023
 * @copyright	License (MIT). Copyright (c) 2023 Storozhenko Roman
 * @brief
 */

/*!****************************************************************************
 * Include
 */
#include "board.h"
#include "hdc1080.h"

enum{
	Temperature = 0x00,		// Temperature measurement output
	Humidity = 0x01,		// Relative Humidity measurement output
	Configuration = 0x02,	// HDC1080 configuration and status
	Serial_ID0 = 0xFB,		// First 2 bytes of the serial ID of the part
	Serial_ID1 = 0xFC,		// Mid 2 bytes of the serial ID of the part
	Serial_ID2 = 0xFD,		// Last byte bit of the serial ID of the part
	Manufacturer_ID = 0xFE,	// 0x5449 ID of Texas Instruments
	Device_ID = 0xFF		// 0x1050 ID of the device
};

/*!****************************************************************************
 * @brief
 */
uint16_t revbyte(uint16_t in){
	return (in & 0xff) << 8 | (in & 0xff00) >> 8;
}

/*!****************************************************************************
 * @brief
 */
bool hdc1080_init(hdc1080dev_t *dev){
	dev->devid = 0x80;

	uint16_t id = 0;
	if(!dev->regRead(dev->devid, Device_ID, &id, 2, 0)){
		return false;
	}
	if(revbyte(id) != 0x1050){

	}

	return true;
}

/*!****************************************************************************
 * @brief
 */
bool hdc1080_readRh(hdc1080dev_t *dev, uint16_t* rh){
	uint16_t reg;
	if(!dev->regRead(dev->devid, Humidity, &reg, 2, 10)){
		return false;
	}
	uint32_t rev = revbyte(reg);
	*rh = (rev * 1000) / 65536;
	return true;
}

/******************************** END OF FILE ********************************/
