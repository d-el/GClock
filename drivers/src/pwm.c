﻿/*!****************************************************************************
 * @file    	pwm.c
 * @author  	Storozhenko Roman - D_EL
 * @version 	V1.0
 * @date    	02.05.2016
 * @copyright 	The MIT License (MIT). Copyright (c) 2020 Storozhenko Roman
 */
/*!****************************************************************************
* Include
*/
#include "stm32g0xx.h"
#include "board.h"
#include "gpio.h"
#include "pwm.h"

/*!****************************************************************************
* @brief
*/
void pwm_init(void){
	gppin_init(GPIOA, 6, alternateFunctionPushPull, pullDisable, 0, 1);
	gppin_init(GPIOB, 0, alternateFunctionPushPull, pullDisable, 0, 1);

	RCC->APBENR1 |= RCC_APBENR1_TIM3EN;						// Clock enable
	RCC->APBRSTR1 |= RCC_APBRSTR1_TIM3RST;					// Timer reset
	RCC->APBRSTR1 &= ~RCC_APBRSTR1_TIM3RST;

	TIM3->PSC = 1 - 1;										// Prescaler
	TIM3->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;		// PWM mode 1 (NORMAL PWM)
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE;							// Output compare preload enable
	TIM3->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1;		// PWM mode 1 (NORMAL PWM)
	TIM3->CCMR2 |= TIM_CCMR2_OC3PE;							// Output compare preload enable
	TIM3->ARR = 3000/*APB1_FREQ / PWM_FREQ*/;						// Auto reload register
	TIM3->CR1 |= TIM_CR1_ARPE;								// TIMx_ARR register is buffered
	TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC3E;			// CH Output Enable
	TIM3->CR1 |= TIM_CR1_CEN;								// Counter enable

	TIM3->CR2 |= TIM_CR2_MMS_1; // Update event is selected as trigger output (TRGO)

	TIM3->CCR1 = TIM3->ARR;
	TIM3->CCR2 = (uint32_t)TIM3->ARR / 4;
}

/*!****************************************************************************
* @brief    set duty cycle  [X_XX %]
*/
void pwm1set(int32_t dc){
	if(dc > TIM3->ARR)
		dc = TIM3->ARR;
	if(dc < 0)
		dc = 0;
	TIM3->CCR1 = dc;
}

void pwm2set(int32_t dc){
	if(dc > TIM3->ARR)
		dc = TIM3->ARR;
	if(dc < 0)
		dc = 0;
	TIM3->CCR3 = dc;
}

/******************************** END OF FILE ********************************/
