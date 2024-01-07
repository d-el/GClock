/*!****************************************************************************
 * @file		hdc1080.h
 * @author		d_el
 * @version		V1.0
 * @date		Dec 18, 2023
 * @copyright	License (MIT). Copyright (c) 2023 Storozhenko Roman
 * @brief
 */

#ifndef hdc1080_H
#define hdc1080_H

#ifdef __cplusplus
extern "C" {
#endif

/*!****************************************************************************
 * Include
 */
#include <stdint.h>
#include <stdbool.h>

/*!****************************************************************************
 * Typedef
 */
typedef struct{
	uint8_t devid;
	bool (*regWrite)(uint8_t devaddr, uint8_t regaddr, const void *src, uint16_t length);
	bool (*regRead)(uint8_t devaddr, uint8_t regaddr, void *dst, uint16_t length, uint16_t delay_ms);
}hdc1080dev_t;

/*!****************************************************************************
 * Function declaration
 */
bool hdc1080_init(hdc1080dev_t *dev);
bool hdc1080_readRh(hdc1080dev_t *dev, uint16_t* rh);

#ifdef __cplusplus
}
#endif

#endif //hdc1080_H
/******************************** END OF FILE ********************************/
