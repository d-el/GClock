/*!****************************************************************************
 * @file		spi.h
 * @author		d_el
 * @version		V1.0
 * @date		05.12.2023
 * @copyright	License (MIT). Copyright (c) 2023 Storozhenko Roman
 * @brief
 */

#ifndef spi_H
#define spi_H

#ifdef __cplusplus
extern "C" {
#endif

/*!****************************************************************************
 * Include
 */
#include <stddef.h>
#include <stdint.h>

/*!****************************************************************************
 * Typedef
 */
typedef void (*spicb_type)(void);

/*!****************************************************************************
 * Function declaration
 */
void spi_init(void);
void spi_send(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif //spi_H
/******************************** END OF FILE ********************************/
