﻿/*!****************************************************************************
 * @file		uart.h
 * @author		d_el
 * @version		V1.5
 * @date		12.12.2017
 * @brief		Driver for uart STM32F3 MCUs
 * @copyright	The MIT License (MIT). Copyright (c) 2021 Storozhenko Roman
 */

#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

/*!****************************************************************************
* Include
*/
#include "stm32g0xx.h"

/*!****************************************************************************
* User define
*/

//UART1
#define		UART1_USE					(1)
#define		UART1_TxBffSz				(32)
#define		UART1_RxBffSz				(32)
#define		UART1_TXIRQPrior			(14)
#define		UART1_PINAFTX				(0)
#define		UART1_PINAFRX				(0)
#define		UART1_PINAFDE				(1)
#define		UART1_HALFDUPLEX			(0)
#define		UART1_DRIVER_ENABLE			(0)

//UART2
#define		UART2_USE					(1)
#define		UART2_TxBffSz				(64)
#define		UART2_RxBffSz				(1024)
#define		UART2_TXIRQPrior			(15)
#define		UART2_PINAFTX				(1)
#define		UART2_PINAFRX				(1)
#define		UART2_HALFDUPLEX			(0)
#define		UART2_DRIVER_ENABLE			(0)

//UART3
#define		UART3_USE					(1)
#define		UART3_TxBffSz				(128)
#define		UART3_RxBffSz				(128)
#define		UART3_TXIRQPrior			(15)
#define		UART3_PINAFTX				(4)
#define		UART3_PINAFRX				(4)
#define		UART3_HALFDUPLEX			(1)
#define		UART3_DRIVER_ENABLE			(0)

/*!****************************************************************************
 * Enumeration
 */

/******************************************************************************
 * Typedef
 */
typedef enum{
	uartTxFree,
	uartTxRun,
	uartTxSuccess,
	uartTxErr
}uartTxState_type;

typedef enum{
	uartRxFree,
	uartRxRun,
	uartRxSuccess,
	uartRxStop,
	uartRxErr
}uartRxState_type;

typedef struct uartStruct{
	USART_TypeDef				*pUart;
	uint8_t						*pTxBff;
	uint8_t						*pRxBff;

	union{
		struct{
			DMA_Channel_TypeDef		*pUartTxDmaCh;
			DMA_Channel_TypeDef		*pUartRxDmaCh;
			volatile uint32_t		*dmaIfcrTx;			///< DMA interrupt flag clear register Tx
			volatile uint32_t		*dmaIfcrRx;			///< DMA interrupt flag clear register Rx
			uint32_t				dmaIfcrMaskTx;		///< DMA interrupt flag clear register mask Tx
			uint32_t				dmaIfcrMaskRx;		///< DMA interrupt flag clear register mask Rx
		};
		struct{
			uint8_t					*pCurrentTx;
			uint8_t					*pEndTx;
			uint8_t					*pCurrentRx;
			uint8_t					*pEndRx;
		};
	};

	void (*txHoock)(struct uartStruct *uart);
	void (*rxHoock)(struct uartStruct *uart);
	uint32_t frequency;
	uint32_t					baudRate;
	volatile uartTxState_type	txState			:8;
	volatile uartRxState_type	rxState			:8;
	uint8_t						halfDuplex		:1;
	uint8_t						driverEnable	:1;
	volatile uint16_t			txCnt;
	volatile uint16_t			rxCnt;
}uart_type;

typedef void (*uartCallback_type)(uart_type *uart);

/*!****************************************************************************
 * Exported variables
 */
#if (UART1_USE > 0)
extern uart_type			*uart1;
#endif //UART1_USE

#if (UART2_USE > 0)
extern uart_type			*uart2;
#endif //UART2_USE

#if (UART3_USE > 0)
extern uart_type			*uart3;
#endif //UART3_USE

/*!****************************************************************************
* Macro functions
*/
#define uartGetRemainTx(uartx)		(uartx->pUartTxDmaCh->CNDTR)
#define uartGetRemainRx(uartx)		(uartx->pUartRxDmaCh->CNDTR)

/*!****************************************************************************
 * Function declaration
 */
void uart_init(uart_type *uartx, uint32_t baudRate);
void uart_deinit(uart_type *uartx);
void uart_setBaud(uart_type *uartx, uint32_t baudRate);
void uart_setCallback(uart_type *uartx, uartCallback_type txHoock, uartCallback_type rxHoock);
void uart_write(uart_type *uartx, const void *src, uint16_t len);
void uart_read(uart_type *uartx, void *dst, uint16_t len);
void uart_stopRead(uart_type *uartStruct);

#ifdef __cplusplus
}
#endif

#endif //UART_H
/******************************** END OF FILE ********************************/
