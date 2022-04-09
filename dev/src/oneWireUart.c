/*!****************************************************************************
 * @file		oneWireUart.c
 * @author		d_el
 * @version		V1.1
 * @date		06.04.2022
 * @brief
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
* @brief    Initialization one wire interface
*/
void ow_init(void){
	uart_init(OW_UART, 9600);	//1WIRE

	// Create Semaphore for UART
	vSemaphoreCreateBinary(oneWireUartSem);
	assert(oneWireUartSem != NULL);

	uart_setCallback(OW_UART, (uartCallback_type)NULL, uartRxHook);
}

/*!****************************************************************************
* @brief    Set One Wire pin to push-pull output
*/
void ow_setOutHi(void){
	gppin_init(pinsMode[GP_DS18B20].p, pinsMode[GP_DS18B20].npin, outPushPull,
			pinsMode[GP_DS18B20].pull, 1, 0);
}

/*!****************************************************************************
* @brief    Set One Wire pin to Open Drain output
*/
void ow_setOutOpenDrain(void){
	gppin_init(pinsMode[GP_DS18B20].p, pinsMode[GP_DS18B20].npin, pinsMode[GP_DS18B20].mode,
			pinsMode[GP_DS18B20].pull, pinsMode[GP_DS18B20].iniState, pinsMode[GP_DS18B20].nAF);
}

/*!****************************************************************************
* @brief    Reset pulse and presence detect
* @param    None
* @retval   owSt_type
*/
owSt_type ow_reset(void){
	BaseType_t  res;

	if(gppin_get(GP_DS18B20) == 0){
		return owShortCircle; //Check on the Short Circle bus
	}

	uart_setBaud(OW_UART, 9600);
	OW_UART->pTxBff[0] = 0xF0;
	uart_read(OW_UART, OW_UART->pRxBff, 1 + 1);
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
* @brief  Write data
* @param  src - pointer to source buffer
* @param  len - number bytes for transmit
* @retval None
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
* @brief  Read data
* @param  dst - pointer to destination buffer
* @param  len - number bytes for receive
* @retval None
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
