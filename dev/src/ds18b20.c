/*!****************************************************************************
 * @file		ds18b20.c
 * @author		Storozhenko Roman - D_EL
 * @version		V2.2
 * @date		03.11.2022
 * @copyright	The MIT License (MIT). Copyright (c) 2020 Storozhenko Roman
 */

/*!****************************************************************************
* Include
*/
#include <stddef.h>
#include "gpio.h"
#include "crc.h"
#include "ds18b20.h"

/*!****************************************************************************
* @brief	Init ds18b20
* @param	rom - slave ID or NULL for skip rom
*/
ds18b20state_type ds18b20Init(const uint8_t rom[8]){
	owSt_type result = ow_reset();
	if(result != 0){
		return (ds18b20state_type)result;
	}

	if(rom == NULL){
		uint8_t readRom[8];
		result = ow_readRom(readRom);
		if(result != 0){
			return (ds18b20state_type)result;
		}
		if(readRom[0] != DS18B20_FAMILY_CODE){
			return ds18b20st_notDs18b20;
		}
	}

	result = ow_selectRom(rom);
	if(result != owOk){
		return (ds18b20state_type)result;
	}

	uint8_t buff[4];
	buff[0] = WRITE_SCRATCHPAD;
	buff[1] = 127;				//TH
	buff[2] = 0;				//TL
	buff[3] = 0x7F;				//12bit 750ms	0.0625
	result = ow_write(buff, sizeof(buff));
	if(result != owOk){
		return (ds18b20state_type)result;
	}

	return ds18b20st_ok;
}

/*!****************************************************************************
* @brief	Read the contents of the scratchpad
* @param	rom - slave ID or NULL for skip rom
* @param	scratchpad - save to
*/
ds18b20state_type ds18b20ReadScratchpad(const uint8_t rom[8], uint8_t scratchpad[9]){
	owSt_type result = ow_reset();
	if(result != 0){
		return (ds18b20state_type)result;
	}
	result = ow_selectRom(rom);
	if(result != owOk){
		return (ds18b20state_type)result;
	}

	const uint8_t functionCommand = READ_SCRATCHPAD;
	result = ow_write(&functionCommand, 1);
	if(result != owOk){
		return (ds18b20state_type)result;
	}

	result = ow_read(scratchpad, 9);
	if(result != owOk){
		return (ds18b20state_type)result;
	}

	uint8_t crc = crc8Calc(&crc1Wire, scratchpad, 9);
	if(crc != 0){
		return ds18b20st_errorCrc;
	}
	return ds18b20st_ok;
}

/*!****************************************************************************
* @brief	Initiates a single temperature conversion
* @param	rom - slave ID or NULL for skip rom
*/
ds18b20state_type ds18b20ConvertTemp(const uint8_t rom[8]){
	owSt_type result = ow_reset();
	if(result != 0){
		return (ds18b20state_type)result;
	}
	result = ow_selectRom(rom);
	if(result != owOk){
		return (ds18b20state_type)result;
	}

	const uint8_t functionCommand = CONVERT_T;
	result = ow_write(&functionCommand, 1);
	if(result != owOk){
		return (ds18b20state_type)result;
	}
	return ds18b20st_ok;
}

/*!****************************************************************************
* @param	rl - low temperature register
* @param	rh - hight temperature register
* @retval	temperature X_XX
*/
uint16_t ds18b20Reg2tmpr(uint8_t rl, uint8_t rh){
	union{
		struct{
			uint8_t		rl;
			uint8_t		rh;
		}byte;
		int16_t	word;
	}scratchpad;

	scratchpad.byte.rl = rl;
	scratchpad.byte.rh = rh;

	return (scratchpad.word * 10U + (16/2)) / 16;	// Div with round
}

/******************************** END OF FILE ********************************/
