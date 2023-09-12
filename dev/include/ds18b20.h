/*!****************************************************************************
 * @file		ds18b20.h
 * @author		Storozhenko Roman - D_EL
 * @version		V2.2
 * @date		03.11.2022
 * @copyright	The MIT License (MIT). Copyright (c) 2020 Storozhenko Roman
 */
#ifndef ds18b20_H
#define ds18b20_H

#ifdef __cplusplus
extern "C" {
#endif

/*!****************************************************************************
* Include
*/
#include <stdint.h>
#include "oneWireUart.h"

/*!****************************************************************************
* User define
*/
#define DS18B20_FAMILY_CODE	0x28
#define READ_SCRATCHPAD		0xBE
#define CONVERT_T			0x44
#define WRITE_SCRATCHPAD	0x4E
#define COPY_SCRATCHPAD		0x48
#define RECALL_E2			0xB8
#define READ_POWER_SUPPLY	0xB4

/*!****************************************************************************
* User typedef
*/
typedef struct{
	uint8_t		integer;
	uint8_t		frac;
	uint8_t		result;
	uint8_t		sign	:1;
	uint8_t		update	:1;
}tmpr_type;

typedef enum{
	ds18b20st_ok,
	ds18b20st_NotFound = owNotFound,
	ds18b20st_ShortCircle = owShortCircle,
	ds18b20st_wTimeOut = owTimeOut,
	ds18b20st_errorCrc = owCrcError,
	ds18b20st_SearchLast = owSearchLast,
	ds18b20st_SearchError = owSearchError,
	ds18b20st_UartTimeout = owUartTimeout,
	ds18b20st_notDs18b20
}ds18b20state_type;

/*!****************************************************************************
* Prototypes for the functions
*/
ds18b20state_type ds18b20Init(const uint8_t rom[8]);
ds18b20state_type ds18b20ReadScratchpad(const uint8_t rom[8], uint8_t scratchpad[9]);
ds18b20state_type ds18b20ConvertTemp(const uint8_t rom[8]);
uint16_t ds18b20Reg2tmpr(uint8_t rl, uint8_t rh);

#ifdef __cplusplus
}
#endif

#endif //ds18b20_H
/******************************** END OF FILE ********************************/
