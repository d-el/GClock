/*!****************************************************************************
 * @file		spi.c
 * @author		d_el
 * @version		V1.0
 * @date		05.12.2023
 * @copyright	License (MIT). Copyright (c) 2023 Storozhenko Roman
 * @brief
 */

/*!****************************************************************************
 * Include
 */
#include <board.h>
#include "gpio.h"
#include "spi.h"

#define DISPLAY_SPI				SPI1

/*!****************************************************************************
 * MEMORY
 */

/*!****************************************************************************
 * @brief SPI initialization
 */
SPI_TypeDef *spi1 = DISPLAY_SPI;

void spi_init(void){
	//Max speed - fPCLK/2
	RCC->APBENR2 |= RCC_APBENR2_SPI1EN;
	RCC->APBRSTR2 |= RCC_APBRSTR2_SPI1RST;
	RCC->APBRSTR2 &= ~RCC_APBRSTR2_SPI1RST;

	spi1->CR1 |= SPI_CR1_BR_1;
	DISPLAY_SPI->CR1 |= SPI_CR1_MSTR;				// Master configuration
	DISPLAY_SPI->CR1 |= SPI_CR1_SSM;				// Software slave management enabled
	DISPLAY_SPI->CR1 |= SPI_CR1_SSI;				// Internal slave select
	DISPLAY_SPI->CR1 &= ~SPI_CR1_LSBFIRST;			// MSB transmitted first
	DISPLAY_SPI->CR2 |= 7 << SPI_CR2_DS_Pos;		// 8 bit data size
	DISPLAY_SPI->CR1 |= SPI_CR1_SPE;				// SPI enable

	gppin_init(GPIOA, 5, alternateFunctionPushPull, pullDisable, 0, 0);	//SPI1_SCK
	//gppin_init(GPIOA, 6, alternateFunctionPushPull, pullDisable, 0, 0);	//SPI1_MISO
	gppin_init(GPIOA, 7, alternateFunctionPushPull, pullDisable, 0, 0);	//SPI1_MOSI
}

/*!****************************************************************************
 * @brief Send data from SPI
 */
void spi_send(uint8_t data){
	*(uint8_t*)(&DISPLAY_SPI->DR) = data;
	//__DSB();
	while((DISPLAY_SPI->SR & SPI_SR_TXE) == 0);
	while((DISPLAY_SPI->SR & SPI_SR_BSY) != 0);
}

/******************************** END OF FILE ********************************/
