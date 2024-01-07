/*!****************************************************************************
 * @file		vfd.h
 * @author		d_el
 * @version		V1.0
 * @date		3.01.2024
 * @copyright	The MIT License (MIT). Copyright (c) 2024 Storozhenko Roman
 * @brief		Driver display
 */

#ifndef st7735_H
#define st7735_H

#ifdef __cplusplus
extern "C" {
#endif


/*!****************************************************************************
 * Include
 */
#include "stdint.h"
#include "stdbool.h"

/*!****************************************************************************
 * Define
 */
#define VFD_W			100
#define VFD_H			14

/*!****************************************************************************
 * Typedef
 */
typedef void (*flushcb_type)(void *arg);
typedef void (*setbufcb_type)(void *arg);

typedef enum {
	black = 0x0000,
	white = 0xFFFF,
	green = 0x07E0,
	blue = 0x001f,
	red = 0xF800,
	yellow = 0xFFE0,
	orange = 0xAB00,
	pink = 0xF97F,
	brown = 0x8200,
	gray = 0x8410,
	lilac = 0x91D4,

	darkGreen = 0x3DA5,
	halfLightGray = 0x39E6,
	halfLightYellow = 0xFFF8,
	halfLightRed = 0xFF18,
	halfLightGreen = 0xC7F8,
	halfLightBlue = 0x861F,
} color_type;

typedef uint16_t lcd_color_type;

/*!****************************************************************************
 * Exported variables
 */
extern uint8_t userframe[40][5];;

static inline void st7735_setPixel(uint16_t x, uint16_t y, lcd_color_type color){
	uint8_t n = (y % 7) * 5 + x % 5 + 5;
	uint8_t *charbit = &userframe[x / 5 + (y / 7) * 20][n / 8];
	if(color){
		*charbit |= 0x80 >> n % 8;
	}else{
		*charbit &= ~(0x80 >> n % 8);
	}
}

/*!****************************************************************************
 * Function declaration
 */
void vfd_init(void);
void vfd_flush(flushcb_type cb);
void vfd_fillBuffer(const lcd_color_type *color, setbufcb_type cb);
void vfd_brightness(uint8_t grid, uint8_t brightness);
void vfd_blinker(uint8_t column, bool val);

#ifdef __cplusplus
}
#endif

#endif //st7735_H
/******************************** END OF FILE ********************************/
