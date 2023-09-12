/*!****************************************************************************
 * @file		write.c
 * @author		d_el
 * @version		V1.0
 * @date		11.11.2022
 * @copyright	The MIT License (MIT). Copyright (c) 2017 Storozhenko Roman
 * @brief		System control task
 */

/*!****************************************************************************
 * Include
 */
#include <reent.h> // required for _write_r
#include "SEGGER_RTT.h"

__attribute__((constructor))
void writeInit(void){
	SEGGER_RTT_Init();
}

int _write(int file, const void *ptr, unsigned int len){
	(void)file;
	SEGGER_RTT_Write(0, ptr, len);
	return len;
}

/******************************** END OF FILE ********************************/
