/*!****************************************************************************
 * @file		debugCore.h
 * @author		d_el - Storozhenko Roman
 * @version		V1.0
 * @date		15.01.2016
 * @copyright	GNU Lesser General Public License v3
 * @brief		Debug utils, HardFault_Handler tracer
 */

/*!****************************************************************************
 * Include
 */
#include "stdint.h"
#include "inttypes.h"
#include "stm32g0xx.h"
#include <plog.h>

/*!****************************************************************************
 * Memory
 */
#define LOG_LOCAL_LEVEL P_LOG_ERROR
static char *logTag = "ARM CORE";

/*!****************************************************************************
 * @brief
 * @retval 1 - debug mode enable
 *         0 - debug mode disable
 */
uint32_t coreIsInDebugMode(void){
	//if((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0){
	//	return 1;   //Debug mode enable
	//}else{
		return 0;   //Debug mode disable
	//}
}

/*!****************************************************************************
 * hard fault handler in C,
 * with stack frame location as input parameter
 * called from HardFault_Handler
 */
void hardFaultHandlerC(unsigned int * stackedContextPtr){
	volatile uint32_t stacked_r0 = stackedContextPtr[0];
	volatile uint32_t stacked_r1 = stackedContextPtr[1];
	volatile uint32_t stacked_r2 = stackedContextPtr[2];
	volatile uint32_t stacked_r3 = stackedContextPtr[3];
	volatile uint32_t stacked_r12 = stackedContextPtr[4];
	volatile uint32_t stacked_lr = stackedContextPtr[5];
	volatile uint32_t stacked_pc = stackedContextPtr[6];
	volatile uint32_t stacked_psr = stackedContextPtr[7];
	volatile SCB_Type *scb = SCB;

	P_LOGE(logTag, "\n\n[GAME OVER]\n");
	P_LOGE(logTag, "R0 = 0x%08" PRIX32, stacked_r0);
	P_LOGE(logTag, "R1 = 0x%08" PRIX32, stacked_r1);
	P_LOGE(logTag, "R2 = 0x%08" PRIX32, stacked_r2);
	P_LOGE(logTag, "R3 = 0x%08" PRIX32, stacked_r3);
	P_LOGE(logTag, "R12 0x%08" PRIX32, stacked_r12);
	P_LOGE(logTag, "LR [R14] 0x%08" PRIX32 " subroutine call return address\n", stacked_lr);
	P_LOGE(logTag, "PC [R15] 0x%08" PRIX32 " program counter\n", stacked_pc);
	P_LOGE(logTag, "PSR 0x%08" PRIX32, stacked_psr);

	// System Handler Control and State Register
	P_LOGE(logTag, "SHCSR 0x%08" PRIX32, scb->SHCSR);

	asm("BKPT #1");
	while(1)
		;
}

/*!****************************************************************************
 *
 */
void HardFault_Handler(void){
	__asm volatile (
			" 		MOVS   R0, #4							\n" /* Determine if processor uses PSP or MSP by checking bit.4 at LR register.		*/
			"		MOV    R1, LR							\n"
			"		TST    R0, R1							\n"
			"		BEQ    _IS_MSP							\n" /* Jump to '_MSP' if processor uses MSP stack.									*/
			"_IS_PSP:                                       \n"
			"		MRS    R0, PSP							\n" /* Prepare PSP content as parameter to the calling function below.				*/
			"		BL	   hardFaultHandlerC      			\n" /* Call 'hardfaultGetContext' passing PSP content as stackedContextPtr value.	*/
			"_IS_MSP:										\n"
			"		MRS    R0, MSP							\n" /* Prepare MSP content as parameter to the calling function below.				*/
			"		BL	   hardFaultHandlerC		        \n" /* Call 'hardfaultGetContext' passing MSP content as stackedContextPtr value.	*/
			:: );
}

/*************** LGPL ************** END OF FILE *********** D_EL ************/
