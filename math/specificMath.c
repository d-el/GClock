﻿/*!****************************************************************************
 * @file    	specificMath.c
 * @author  	Storozhenko Roman - D_EL
 * @version 	V1.0
 * @date    	12-09-2015
 * @copyright 	The MIT License (MIT). Copyright (c) 2020 Storozhenko Roman
 */

/*!****************************************************************************
* Include
*/
#include "specificMath.h"

/*!****************************************************************************
*
*/
_iq iq_Fy_x1x2y1y2x(_iq x1, _iq x2, _iq y1, _iq y2, _iq x)
{
    _iq     df, dx;
    df = y2-y1;
    dx = x2-x1;
    if(dx == 0){
        return 0x7FFFFFFFL;
    }
    return y1 + _IQmpy((_IQdiv(df, dx)), (x - x1));
}

/*!****************************************************************************
*
*/
_iq s32iq_Fy_x1x2y1y2x(int32_t x1, int32_t x2, _iq y1, _iq y2, int32_t x){
    _iq     df;
    int32_t dx;

    df = y2-y1;
    dx = x2-x1;
    if(dx == 0){
        return 0x7FFFFFFFL;
    }
    return y1 + ((int64_t)df * ((x - x1))) / dx;
}

/*!****************************************************************************
*
*/
int32_t iqs32_Fy_x1x2y1y2x(_iq x1, _iq x2, int32_t y1, int32_t y2, _iq x){
    int32_t df;
    _iq     dx;

    df = y2-y1;
    dx = x2-x1;
    if(dx == 0){
        return INT32_MAX;
    }
	return y1 + ((int64_t)df * ((x - x1))) / dx;
}

/******************************** END OF FILE ********************************/
