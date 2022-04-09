/*!****************************************************************************
 * @file		ds18b20.h
 * @author		Storozhenko Roman - D_EL
 * @version		V1.0
 * @date		09.04.2022
 * @copyright 	The MIT License (MIT). Copyright (c) 2022 Storozhenko Roman
 */
#ifndef DATECS_H_
#define DATECS_H_

#include <stdint.h>
#include <stddef.h>

class Datecs{
public:
	using Interface = void(*)(const void* c, size_t len);
public:
	Datecs() = delete;
	Datecs(Interface i);
	void init();
	void brightness(uint8_t level);	// n = 1, Brightness Level = 40 %
									// n = 2, Brightness Level = 60 %
									// n = 3, Brightness Level = 80 %
									// n = 4, Brightness Level = 100 %
	void clear();
	void putstring(uint8_t x, uint8_t y, const char* s);
	void flush();

private:
	char frame[2][20];
	Interface interface;
};

#endif /* DATECS_H_ */
