/*!****************************************************************************
 * @file		vfd.c
 * @author		d_el
 * @version		V1.0
 * @date		3.01.2024
 * @copyright	The MIT License (MIT). Copyright (c) 2024 Storozhenko Roman
 * @brief		Driver display
 */

/*!****************************************************************************
 * Include
 */
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <gpio.h>
#include <spi.h>
#include <board.h>
#include "vfd.h"

static SemaphoreHandle_t dispSem;
flushcb_type fcb;

bool userflush;
uint8_t userframe[40][5];
uint8_t frame[40][5];

uint8_t gridmap[40] = {
		0, 39, 1, 38, 2, 37, 3, 36, 4, 35, 5, 34, 6, 33, 7, 32, 8, 31, 9, 30, 10, 29,
		11, 28, 12, 27, 13, 26, 14, 25, 15, 24, 16, 23, 17, 22, 18, 21, 19, 20
};

uint8_t brightnessmap[40] = {
	100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100
};

void disp_tim_init(){
	gppin_init(GPIOA, 6, alternateFunctionPushPull, pullDisable, 0, 1);
	RCC->APBENR1 |= RCC_APBENR1_TIM3EN;
	RCC->APBRSTR1 |= RCC_APBRSTR1_TIM3RST;
	RCC->APBRSTR1 &= ~RCC_APBRSTR1_TIM3RST;
	TIM3->PSC		= APB1_FREQ / (1000000) - 1;					// Set prescaler
	TIM3->CR1		|= TIM_CR1_ARPE;							// TIMx_ARR register is buffered
	TIM3->CR2		|= TIM_CR2_MMS_2;							// Compare - OC1REF signal is used as trigger output (TRGO)
	TIM3->CCMR1		|= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0;		//PWM mode 1 (NORMAL PWM)
	TIM3->CCR1 		= 10;
	TIM3->CCER		|= TIM_CCER_CC1E;							// Channel enable
	TIM3->ARR		= 100;										// Arr value, [us]
	TIM3->DIER 		|= TIM_DIER_UIE;

	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_SetPriority(TIM3_IRQn, 15);

	TIM3->CR1		|= TIM_CR1_CEN;
}

void TIM3_IRQHandler(void){
	TIM3->SR		= ~TIM_SR_UIF;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(dispSem, &xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

void dispTSK(void *pPrm){
	vSemaphoreCreateBinary(dispSem);
	xSemaphoreTake(dispSem, portMAX_DELAY);

	spi_init();
	disp_tim_init();
	gppin_reset(GP_VFD_LS);

	uint8_t grid = 0;
	uint8_t gridnum = 40;

	while(1){
		xSemaphoreTake(dispSem, portMAX_DELAY);

		gppin_init(GPIOA, 6, outPushPull, pullDisable, 0, 0);
		gppin_set(GP_VFD_LS);
		__NOP();
		__NOP();
		__NOP();
		gppin_reset(GP_VFD_LS);
		gppin_init(GPIOA, 6, alternateFunctionPushPull, pullDisable, 0, 1);

		if(userflush){
			memcpy(frame, userframe, sizeof(frame));
			if(fcb){
				fcb(NULL);
			}
			userflush = false;
		}

		uint8_t mmap = gridmap[grid];
		TIM3->CCR1 = (100 - brightnessmap[(39 - mmap)]) + 15;

		uint8_t dspreg[10] = {};
		dspreg[mmap / 8] = 0x80 >> mmap % 8;

		for(int i = 0; i < 5; i++){
			dspreg[5 + i] = frame[mmap][i];
		}

		for(int i = 0; i < 10; i++){
			spi_send(dspreg[i]);
		}

		grid++;
		if(grid == gridnum){
			grid = 0;
		}
	}
}

void vfd_init(void){
	xTaskCreate(dispTSK, "dispTSK", VFD_TSK_SZ_STACK, NULL, VFD_TSK_PRIO, NULL);
}

void vfd_flush(flushcb_type cb){
	fcb = cb;
	userflush = true;
}

void vfd_fillBuffer(const lcd_color_type *color, setbufcb_type cb){
	for(uint8_t i = 0; i < 40; i++){
		userframe[i][0] = color ? 0x07 : 0x00 | (userframe[i][0] & 0x08);
		for(uint8_t j = 1; j < 5; j++){
			userframe[i][j] = color ? 0xFF : 0x00;;
		}
	}
	cb(NULL);
}

void vfd_brightness(uint8_t grid, uint8_t brightness){
	if(grid < 40){
		brightnessmap[grid] = brightness;
	}
	if(grid == 99){
		memset(&brightnessmap[0], brightness, sizeof(brightnessmap));
	}
}

void vfd_blinker(uint8_t column, bool val){
	if(column > 19){
		return;
	}
	if(val){
		userframe[column + 20][0] |= 0x08;
	}else{
		userframe[column + 20][0] &= ~0x08;
	}
}

/******************************** END OF FILE ********************************/
