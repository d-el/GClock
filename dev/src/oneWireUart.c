/*!****************************************************************************
 * @file		oneWireUart.c
 * @author		d_el
 * @version		V1.2
 * @date		03.11.2022
 * @copyright	The MIT License (MIT). Copyright (c) 2022 Storozhenko Roman
 */

/*!****************************************************************************
* Include
*/
#include <assert.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <uart.h>
#include <gpio.h>
#include "oneWireUart.h"
#include <crc.h>

/*!****************************************************************************
* MEMORY
*/
static SemaphoreHandle_t oneWireUartSem;

/*!****************************************************************************
 * @brief	uart RX callback
 */
static void uartRxHook(uart_type *puart){
	(void)puart;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(oneWireUartSem, &xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

/*!****************************************************************************
* @brief	Initialization one wire interface
*/
void ow_init(void){
	uart_init(OW_UART, 9600);	//1WIRE

	// Create Semaphore for UART
	vSemaphoreCreateBinary(oneWireUartSem);
	assert(oneWireUartSem != NULL);
	xSemaphoreTake(oneWireUartSem, portMAX_DELAY);

	uart_setCallback(OW_UART, (uartCallback_type)NULL, uartRxHook);
}

/*!****************************************************************************
* @brief	Set One Wire pin to push-pull output
*/
void ow_setOutHi(void){
	gppin_init(pinsMode[GP_DS18B20].p, pinsMode[GP_DS18B20].npin, outPushPull,
			pinsMode[GP_DS18B20].pull, 1, 0);
}

/*!****************************************************************************
* @brief	Set One Wire pin to Open Drain output
*/
void ow_setOutOpenDrain(void){
	gppin_init(pinsMode[GP_DS18B20].p, pinsMode[GP_DS18B20].npin, pinsMode[GP_DS18B20].mode,
			pinsMode[GP_DS18B20].pull, pinsMode[GP_DS18B20].iniState, pinsMode[GP_DS18B20].nAF);
}

/*!****************************************************************************
* @brief	Reset pulse and presence detect
* @param	None
* @retval	owSt_type
*/
owSt_type ow_reset(void){
	BaseType_t  res;

	if(gppin_get(GP_DS18B20) == 0){
		return owShortCircle; //Check on the Short Circle bus
	}

	uart_setBaud(OW_UART, 9600);
	OW_UART->pTxBff[0] = 0xF0;
	xSemaphoreTake(oneWireUartSem, 0);
	uart_read(OW_UART, OW_UART->pRxBff, 1);
	uart_write(OW_UART, OW_UART->pTxBff, 1);
	res = xSemaphoreTake(oneWireUartSem, pdMS_TO_TICKS(OW_TIMEOUT));
	if(res != pdTRUE){
		return owUartTimeout;
	}

	if(((OW_UART->pRxBff[0] >= 0x90)&&(OW_UART->pRxBff[0] <= 0xE0))||(OW_UART->pRxBff[0] == 0)){
		return owOk;
	}else{
		return owNotFound;
	}
}

/*!***************************************************************************
* @brief	Write one bit
* @param	bit value
* @retval 	None
*/
owSt_type ow_writebit(uint8_t src){
	uint8_t *pBff = OW_UART->pTxBff;
	uint8_t byteTrans = 1;
	if(src != 0){
		*pBff = 0xFF;
	}else{
		*pBff = 0x00;
	}
	uart_setBaud(OW_UART, 115200);
	uart_read(OW_UART, OW_UART->pRxBff, byteTrans);
	uart_write(OW_UART, OW_UART->pTxBff, byteTrans);
	BaseType_t res = xSemaphoreTake(oneWireUartSem, pdMS_TO_TICKS(OW_TIMEOUT));
	if(res != pdTRUE){
		return owUartTimeout;
	}
	return owOk;
}

/*!***************************************************************************
* @brief	Write data
* @param	src - pointer to source buffer
* @param	len - number bytes for transmit
* @retval	None
*/
owSt_type ow_write(const void *src, uint8_t len){
	uint8_t *pSrc		= (uint8_t*)src;
	uint8_t *pSrcEnd	= pSrc + len;
	uint8_t *pBff		= OW_UART->pTxBff;
	uint8_t mask, byteTrans = len << 3;

	while(pSrc < pSrcEnd){
		for(mask = 1; mask != 0; mask <<= 1){
			if((*pSrc & mask) != 0){
				*pBff++ = 0xFF;
			}else{
				*pBff++ = 0x00;
			}
		}
		pSrc++;
	}

	uart_setBaud(OW_UART, 115200);
	uart_read(OW_UART, OW_UART->pRxBff, byteTrans);
	uart_write(OW_UART, OW_UART->pTxBff, byteTrans);
	BaseType_t res = xSemaphoreTake(oneWireUartSem, pdMS_TO_TICKS(OW_TIMEOUT));
	if(res != pdTRUE){
		return owUartTimeout;
	}
	return owOk;
}

/*!***************************************************************************
* @brief	Read one bit
* @param	dst - pointer to destination bt value
* @retval	Status operation
*/
owSt_type ow_readbit(uint8_t *dst){
	uint8_t	 byteTrans = 1;
	memset(OW_UART->pTxBff, 0xFF, byteTrans);
	uart_setBaud(OW_UART, 115200);
	uart_read(OW_UART, OW_UART->pRxBff, byteTrans);
	uart_write(OW_UART, OW_UART->pTxBff, byteTrans);
	BaseType_t res = xSemaphoreTake(oneWireUartSem, pdMS_TO_TICKS(OW_TIMEOUT));

	if(res == pdTRUE){
		if(OW_UART->pRxBff[0] == 0xFF){
			*dst = 1; //Read '1'
		}else{
			*dst = 0;
		}
	}
	else{
		return owUartTimeout;
	}

	return owOk;
}

/*!***************************************************************************
* @brief	Read data
* @param	dst - pointer to destination buffer
* @param	len - number bytes for receive
* @retval	Status operation
*/
owSt_type ow_read(void *dst, uint8_t len){
	BaseType_t  res;
	uint8_t		*pDst		= dst;
	uint8_t		*pDstEnd	= pDst + len;
	uint8_t		*pBff		= OW_UART->pRxBff;
	uint8_t		mask, byteTrans = len << 3;

	memset(OW_UART->pTxBff, 0xFF, byteTrans);

	uart_setBaud(OW_UART, 115200);
	uart_read(OW_UART, OW_UART->pRxBff, byteTrans);
	uart_write(OW_UART, OW_UART->pTxBff, byteTrans);
	res = xSemaphoreTake(oneWireUartSem, pdMS_TO_TICKS(OW_TIMEOUT));

	if(res == pdTRUE){
		while(pDst < pDstEnd){
			*pDst = 0;
			for(mask = 1; mask != 0; mask <<= 1){
				if(*pBff++ == 0xFF){
					*pDst |= mask; //Read '1'
				}
			}
			pDst++;
		}
	}
	else{
		return owUartTimeout;
	}

	return owOk;
}

/*!***************************************************************************
 * @brief	Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing search state
 * @param	Searching context
 * @retval	Status operation
 */
owSt_type ow_searchRom(ow_searchRomContext_t* context){
	owSt_type searchResult = owOk;
	uint8_t lastZero = 0, idBitNumber = 1, romByteNumber = 0/*, searchResult = 0*/;
	uint8_t idBit, cmpIdBit;
	uint8_t romByteMask = 1, searchDirection;

	// If the last call was not the last one
	if(!context->lastDeviceFlag){
		if(ow_reset() != owOk){
			// Reset the search
			context->lastDiscrepancy = 0;
			context->lastDeviceFlag = 0;
			context->lastFamilyDiscrepancy = 0;
			return owNotFound;
		}

		// Issue the search command
		const uint8_t search = SEARCH_ROM;
		ow_write(&search, 1);
		// loop to do the search
		do{
			// Read a bit and its complement
			ow_readbit(&idBit);
			ow_readbit(&cmpIdBit);

			// Check for no devices on 1-wire
			if((idBit == 1) && (cmpIdBit == 1)){
				break;
			}else{
				// All devices coupled have 0 or 1
				if(idBit != cmpIdBit){
					searchDirection = idBit; // bit write value for search
				}else{
					// If this discrepancy if before the Last Discrepancy
					// On a previous next then pick the same as last time
					if(idBitNumber < context->lastDiscrepancy){
						searchDirection = ((context->rom[romByteNumber] & romByteMask) > 0);
					}else{
						// If equal to last pick 1, if not then pick 0
						searchDirection = (idBitNumber == context->lastDiscrepancy);
					}
					// If 0 was picked then record its position in LastZero
					if(searchDirection == 0){
						lastZero = idBitNumber;
						// Check for Last discrepancy in family
						if(lastZero < 9)
							context->lastFamilyDiscrepancy = lastZero;
					}
				}
				// Set or clear the bit in the ROM byte rom_byte_number
				// With mask rom_byte_mask
				if(searchDirection == 1){
					context->rom[romByteNumber] |= romByteMask;
				}else{
					context->rom[romByteNumber] &= ~romByteMask;
				}
				// Serial number search direction write bit
				ow_writebit(searchDirection);
				// Increment the byte counter id_bit_number
				// And shift the mask rom_byte_mask
				idBitNumber++;
				romByteMask <<= 1;
				// If the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if(romByteMask == 0){
					romByteNumber++;
					romByteMask = 1;
				}
			}
		}while(romByteNumber < 8); // Loop until through all ROM bytes 0-7
		// If the search was successful then
		if(!((idBitNumber < 65) || (crc8Calc(&crc1Wire, context->rom, 8) != 0))){
			// Search successful so set LastDiscrepancy, LastDeviceFlag, search_result
			context->lastDiscrepancy = lastZero;
			// Check for last device
			if(context->lastDiscrepancy == 0){
				context->lastDeviceFlag = 1;
				searchResult = owSearchLast;
			}else{
				searchResult = owSearchOk;
			}

		}
	}
	// If no device found then reset counters so next 'search' will be like a first
	if(!searchResult || !context->rom[0]){
		context->lastDiscrepancy = 0;
		context->lastDeviceFlag = 0;
		context->lastFamilyDiscrepancy = 0;
		searchResult = owNotFound;
	}
	return searchResult;
}

/*!***************************************************************************
 * @brief	Read the slave’s 64-bit ROM code
 * @param	rom - destination
 * @retval	Status operation
 */
owSt_type ow_readRom(uint8_t rom[8]){
	uint8_t buff[8];
	buff[0] = READ_ROM;
	owSt_type result = ow_write(buff, 1);
	if(result != owOk){
		return result;
	}

	result = ow_read(rom, 8);
	if(result != owOk){
		return result;
	}

	uint8_t crc = crc8Calc(&crc1Wire, rom, 8);
	if(crc != 0){
		return owCrcError;
	}
	return owOk;
}

/*!***************************************************************************
 * @brief	Address a specific slave device
 * @param	rom - slave ID or NULL for skip rom
 * @retval	Status operation
 */
owSt_type ow_selectRom(const uint8_t rom[8]){
	uint8_t buff[9];
	uint8_t size = 0;

	if(rom != NULL){
		buff[size++] = MATCH_ROM;
		for(size_t i = 0; i < 8; i++){
			buff[size++] = rom[i];
		}
	}else{
		buff[size++] = SKIP_ROM;
	}
	owSt_type result = ow_write(buff, size);
	return result;
}

/*!***************************************************************************
 *  CRC8 MAXIM
 * Polynomial:		0x31
 * Initial Value:	0x00
 * Final Xor Value:	0x00
 */
uint8_t ow_crc8(uint8_t *mas, uint8_t n){
	uint8_t j , i, tmp, data, crc = 0;

	for(i = 0; i < n; i++){
		data = *mas;
		for(j = 0; j < 8; j++){
			tmp = (crc ^ data) & 0x01;
			if (tmp == 0x01) crc = crc ^ 0x18;
			crc = (crc >> 1) & 0x7F;
			if (tmp == 0x01) crc = crc | 0x80;
			data = data >> 1;
		}
		mas++;
	}
	return crc;
}

/******************************** END OF FILE ********************************/
