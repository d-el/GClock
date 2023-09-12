/*!****************************************************************************
 * @file		oneWireUart.h
 * @author		d_el
 * @version		V1.2
 * @date		03.11.2022
 * @copyright	The MIT License (MIT). Copyright (c) 2022 Storozhenko Roman
 */
#ifndef oneWireUart_H
#define oneWireUart_H

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
#define OW_UART				(uart3)
#define OW_TIMEOUT			(50)		//[ms]

// ROM COMMANDS Definition
#define SEARCH_ROM			0xF0
#define MATCH_ROM			0x55
#define READ_ROM			0x33
#define SKIP_ROM			0xCC
#define ALARM_SEARCH		0xEC

/*!****************************************************************************
* User enum
*/

/*!****************************************************************************
* User typedef
*/
typedef enum{
	owOk,
	owNotFound,
	owShortCircle,
	owTimeOut,
	owCrcError,
	owSearchOk,
	owSearchLast,
	owSearchError,
	owUartTimeout
}owSt_type;

typedef struct{
	uint8_t rom[8];
	uint8_t lastDiscrepancy;
	uint8_t lastFamilyDiscrepancy;
	uint8_t lastDeviceFlag;
}ow_searchRomContext_t;

/*!****************************************************************************
* Macro functions
*/

/*!****************************************************************************
* Prototypes for the functions
*/
void ow_init(void);
void ow_setOutHi(void);
void ow_setOutOpenDrain(void);
owSt_type ow_reset(void);
owSt_type ow_write(const void *src, uint8_t len);
owSt_type ow_read(void *dst, uint8_t len);
owSt_type ow_searchRom(ow_searchRomContext_t* context);
owSt_type ow_readRom(uint8_t rom[8]);
owSt_type ow_selectRom(const uint8_t rom[8]);
uint8_t ow_crc8(uint8_t *mas, uint8_t n);

#ifdef __cplusplus
}
#endif

#endif //oneWireUart_H
/******************************** END OF FILE ********************************/
