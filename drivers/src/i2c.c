/*!****************************************************************************
 * @file    	i2c.c
 * @author  	Storozhenko Roman - D_EL
 * @version 	V1.2
 * @date    	22.11.2015
 * @copyright 	The MIT License (MIT). Copyright (c) 2020 Storozhenko Roman
 */

/*!****************************************************************************
* Include
*/
#include "gpio.h"
#include "i2c.h"

/*!****************************************************************************
* i2c1 memory
*/
#if (I2C1_USE > 0)
i2c_type		i2c1Sct;
i2c_type		*i2c1 = &i2c1Sct;
uint8_t			i2c1TxBff[I2C1_TxBffSz];
uint8_t			i2c1RxBff[I2C1_RxBffSz];
#endif //I2C1_USE

/*!****************************************************************************
* i2c2 memory
*/
#if (I2C2_USE > 0)
i2c_type		i2c2Sct;
i2c_type		*i2c2 = &i2c2Sct;
uint8_t			i2c2TxBff[I2C2_TxBffSz];
uint8_t			i2c2RxBff[I2C2_RxBffSz];
#endif //I2C2_USE

/*!****************************************************************************
*
*/
void i2c_init(i2c_type *i2cx){
	#if (I2C1_USE > 0)
	if(i2cx == i2c1){
		/************************************************
		* Memory setting
		*/
		i2cx->pI2c			 = I2C1;
		i2cx->pDmaTxDmaCh	= DMA1_Channel4;
		i2cx->pDmaRxDmaCh	= DMA1_Channel5;

		/************************************************
		* IO
		* PB6 - I2C2_CSCL
		* PB7 - I2C2_SDA
		*/
		gppin_init(GPIOB, 7, alternateFunctionOpenDrain, pullUp, 0, I2C1_PINAFSCL);
		gppin_init(GPIOC, 14, alternateFunctionOpenDrain, pullUp, 0, I2C1_PINAFSDA);

		/************************************************
		* NVIC
		*/
		NVIC_EnableIRQ(I2C1_IRQn);											//Event interrupt enable
		NVIC_SetPriority(I2C1_IRQn, I2C1_EventInterruptPrior);

		/************************************************
		* I2C clock
		*/
		RCC->APBENR1	|= RCC_APBENR1_I2C1EN;									//Clock enable
		RCC->APBRSTR1	|= RCC_APBRSTR1_I2C1RST;								//Reset
		RCC->APBRSTR1	&= ~RCC_APBRSTR1_I2C1RST;

		/************************************************
		* DMA clock
		*/
		RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	}
	#endif //I2C2_USE

	#if (I2C2_USE > 0)
	if(i2cx == i2c2){
		/************************************************
		* Memory setting
		*/
		i2cx->pI2c			= I2C2;
		i2cx->pDmaTxDmaCh	= DMA1_Channel4;
		i2cx->pDmaRxDmaCh	= DMA1_Channel5;

		/************************************************
		* IO
		* PB13 - I2C2_CSCL
		* PB14 - I2C2_SDA
		*/
		gppin_init(GPIOB, 13, alternateFunctionOpenDrain, pullUp, 0, I2C2_PINAFSCL);
		gppin_init(GPIOB, 14, alternateFunctionOpenDrain, pullUp, 0, I2C2_PINAFSDA);

		/************************************************
		* NVIC
		*/
		NVIC_EnableIRQ(I2C2_EV_IRQn);											//Event interrupt enable
		NVIC_SetPriority(I2C2_EV_IRQn, I2C1_EventInterruptPrior);
		NVIC_EnableIRQ(I2C2_ER_IRQn);											//Error interrupt enable
		NVIC_SetPriority(I2C2_ER_IRQn, I2C1_ErrorInterruptPrior);

		/************************************************
		* I2C clock
		*/
		RCC->APB1ENR	|= RCC_APB1ENR_I2C2EN;									//USART1 clock enable
		RCC->APB1RSTR	|= RCC_APB1RSTR_I2C2RST;								//USART1 reset
		RCC->APB1RSTR	&= ~RCC_APB1RSTR_I2C2RST;

		/************************************************
		* DMA clock
		*/
		RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	}
	#endif //I2C2_USE

	/************************************************
	* I2C
	*/
	i2cx->pI2c->CR1	|= I2C_CR1_TXDMAEN;								//DMA mode disabled for transmission
	i2cx->pI2c->CR1	|= I2C_CR1_RXDMAEN;								//DMA mode enabled for reception
	i2cx->pI2c->CR1	|= I2C_CR1_STOPIE;								//Stop detection (STOPF) interrupt enabled
	i2cx->pI2c->CR2	|= I2C_CR2_AUTOEND;								//Automatic end mode: a STOP condition is automatically sent when NBYTES data are transferred

	//I2C_TIMINGR 400 kHz for 8 MHz Peripheral clock	(ref DocID027295 Rev 2 1024/1426)
	i2cx->pI2c->TIMINGR |= (0x0) << I2C_TIMINGR_PRESC_Pos;
	i2cx->pI2c->TIMINGR |= (0x9) << I2C_TIMINGR_SCLL_Pos;
	i2cx->pI2c->TIMINGR |= (0x3) << I2C_TIMINGR_SCLH_Pos;
	i2cx->pI2c->TIMINGR |= (0x1) << I2C_TIMINGR_SDADEL_Pos;
	i2cx->pI2c->TIMINGR |= (0x3) << I2C_TIMINGR_SCLDEL_Pos;

	i2cx->pI2c->CR1	|= I2C_CR1_PE;			//Peripheral Enable

	/************************************************
	* DMA
	*/
	//DMA Channel I2C TX
	i2cx->pDmaTxDmaCh->CCR = 0;
	i2cx->pDmaTxDmaCh->CCR &= ~DMA_CCR_EN;										//Channel disabled
	i2cx->pDmaTxDmaCh->CCR |= DMA_CCR_PL_0;										//Channel priority level - Medium
	i2cx->pDmaTxDmaCh->CCR &= ~DMA_CCR_MSIZE;									//Memory size - 8 bit
	i2cx->pDmaTxDmaCh->CCR &= ~DMA_CCR_PSIZE;									//Peripheral size - 8 bit
	i2cx->pDmaTxDmaCh->CCR |= DMA_CCR_MINC;										//Memory increment mode enabled
	i2cx->pDmaTxDmaCh->CCR &= ~DMA_CCR_PINC;									//Peripheral increment mode disabled
	i2cx->pDmaTxDmaCh->CCR &= ~DMA_CCR_CIRC;									//Circular mode disabled
	i2cx->pDmaTxDmaCh->CCR |= DMA_CCR_DIR;										//Read from memory
	i2cx->pDmaTxDmaCh->CNDTR = 0;												//Number of data
	i2cx->pDmaTxDmaCh->CPAR = (uint32_t)&i2cx->pI2c->TXDR;						//Peripheral address
	i2cx->pDmaTxDmaCh->CMAR = (uint32_t)NULL;									//Memory address

	//DMA Channel I2C RX
	i2cx->pDmaRxDmaCh->CCR = 0;
	i2cx->pDmaRxDmaCh->CCR &= ~DMA_CCR_EN;										//Channel disabled
	i2cx->pDmaRxDmaCh->CCR |= DMA_CCR_PL_0;										//Channel priority level - Medium
	i2cx->pDmaRxDmaCh->CCR &= ~DMA_CCR_MSIZE;									//Memory size - 8 bit
	i2cx->pDmaRxDmaCh->CCR &= ~DMA_CCR_PSIZE;									//Peripheral size - 8 bit
	i2cx->pDmaRxDmaCh->CCR |= DMA_CCR_MINC;										//Memory increment mode enabled
	i2cx->pDmaRxDmaCh->CCR &= ~DMA_CCR_PINC;									//Peripheral increment mode disabled
	i2cx->pDmaRxDmaCh->CCR &= ~DMA_CCR_CIRC;									//Circular mode disabled
	i2cx->pDmaRxDmaCh->CCR &= ~DMA_CCR_DIR;										//Read from peripheral
	i2cx->pDmaRxDmaCh->CCR |= DMA_CCR_TCIE;										//Transfer complete interrupt enable
	i2cx->pDmaRxDmaCh->CNDTR = 0;												//Number of data
	i2cx->pDmaRxDmaCh->CPAR = (uint32_t)&i2cx->pI2c->RXDR;						//Peripheral address
	i2cx->pDmaRxDmaCh->CMAR = (uint32_t)NULL;									//Memory address
}

/*!****************************************************************************
*
*/
void i2c_reInit(i2c_type *i2cx){
	i2c_init(i2cx);
}

/*!****************************************************************************
 * @brief	Set callback I2C
 */
void i2c_setCallback(i2c_type *i2cx, i2cCallback_type tcHook){
	i2cx->tcHook = tcHook;
}

/*!****************************************************************************
*
*/
void i2c_write(i2c_type *i2cx, void *src, uint16_t len, uint8_t slaveAdr, i2c_stopMode_type stopMode){
	i2cx->pDmaTxDmaCh->CCR		&= ~DMA_CCR_EN;
	i2cx->pDmaTxDmaCh->CMAR		= (uint32_t)src;
	i2cx->pDmaTxDmaCh->CNDTR	= len;
	i2cx->slaveAdr				= slaveAdr;
	i2cx->stopMode				= stopMode;

	if(stopMode == i2cNeedStop){
		i2cx->pI2c->CR2	|= I2C_CR2_AUTOEND;					//Automatic end mode: a STOP condition is automatically sent when NBYTES data are transferred
		i2cx->pI2c->CR1	&= ~I2C_CR1_TXIE;					//TX interrupt disable
	}else{
		i2cx->pI2c->CR2	&= ~I2C_CR2_AUTOEND;
		i2cx->pI2c->CR1	|= I2C_CR1_TXIE;					//TX interrupt enable
	}

	i2cx->pI2c->CR2	&= ~I2C_CR2_RD_WRN;						//Master requests a write transfer
	i2cx->pI2c->CR2	&= ~I2C_CR2_NBYTES;
	i2cx->pI2c->CR2	|= (len) << I2C_CR2_NBYTES_Pos;			//Number of bytes
	i2cx->pI2c->CR2	&= ~I2C_CR2_SADD;						//Slave address bit (master mode) clear
	i2cx->pI2c->CR2	|= (slaveAdr) << I2C_CR2_SADD_Pos;		//Slave address bit (master mode)

	i2cx->pDmaTxDmaCh->CCR		|= DMA_CCR_EN;
	i2cx->pI2c->CR2				|= I2C_CR2_START;			//Generation start condition
	i2cx->state					= i2cTxRun;
}

/*!****************************************************************************
*
*/
void i2c_read(i2c_type *i2cx, void *dst, uint16_t len, uint8_t slaveAdr){
	i2cx->pDmaRxDmaCh->CCR		&= ~DMA_CCR_EN;
	i2cx->pDmaRxDmaCh->CMAR		= (uint32_t)dst;
	i2cx->pDmaRxDmaCh->CNDTR	= len;
	i2cx->slaveAdr				= slaveAdr;

	i2cx->pI2c->CR2	|= I2C_CR2_AUTOEND;						//Automatic end mode: a STOP condition is automatically sent when NBYTES data are transferred
	i2cx->pI2c->CR1	&= ~I2C_CR1_TXIE;						//TX interrupt disable

	i2cx->pI2c->CR2		|= I2C_CR2_RD_WRN;					//Master requests a read transfer

	i2cx->pI2c->CR2	&= ~I2C_CR2_NBYTES;
	i2cx->pI2c->CR2	|= (len) << I2C_CR2_NBYTES_Pos;			//Number of bytes

	i2cx->pI2c->CR2	&= (~(slaveAdr)) << I2C_CR2_SADD_Pos;	//Slave address bit (master mode)
	i2cx->pI2c->CR2	|= (slaveAdr) << I2C_CR2_SADD_Pos;		//Slave address bit (master mode)

	i2cx->pDmaRxDmaCh->CCR		|= DMA_CCR_EN;
	i2cx->pI2c->CR2				|= I2C_CR2_START;			//Generation start condition
	i2cx->state					= i2cRxRun;
}

/******************************************************************************
* I2C EVENT HANDLER
*/
void i2cEventHendler(i2c_type *i2cx){
	if((i2cx->pI2c->ISR & I2C_ISR_STOPF) != 0){
		if(i2cx->state == i2cTxRun){
			i2cx->state = i2cTxSuccess;
		}
		if(i2cx->state == i2cRxRun){
			i2cx->state = i2cRxSuccess;
		}
		if(i2cx->tcHook != NULL){				//Call hook
			i2cx->tcHook(i2cx);
		}
		i2cx->pI2c->ICR = I2C_ICR_STOPCF;
	}
	else if((i2cx->pI2c->ISR & I2C_ISR_TXE) != 0){
		i2cx->state = i2cTxSuccess;
		if(i2cx->tcHook != NULL){				//Call hook
			i2cx->tcHook(i2cx);
		}
		i2cx->pI2c->ICR = I2C_ICR_STOPCF;
	}
}

/******************************************************************************
* I2C ERROR HANDLER
*/
void i2cErrorHendler(i2c_type *i2cx){
	(void)i2cx->pI2c->ISR;
}

/******************************************************************************
* I2C1
*/
#if (I2C1_USE > 0)
void I2C1_EV_IRQHandler(void){
	i2cEventHendler(i2c1);
}
void I2C1_ER_IRQHandler(void){
	i2cErrorHendler(i2c1);
}
#endif

/******************************************************************************
* I2C2
*/
#if (I2C2_USE > 0)
void I2C2_EV_IRQHandler(void){
	i2cEventHendler(i2c2);
}
void I2C2_ER_IRQHandler(void){
	i2cErrorHendler(i2c2);
}
#endif

/******************************** END OF FILE ********************************/
