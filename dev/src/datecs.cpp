/*!****************************************************************************
 * @file		datecs.cpp
 * @author		Storozhenko Roman - D_EL
 * @version		V1.0
 * @date		09.04.2022
 * @copyright 	The MIT License (MIT). Copyright (c) 2022 Storozhenko Roman
 */

#include "datecs.h"
#include <string.h>

Datecs& Datecs::get(){
	static Datecs d;
	return d;
}

void Datecs::setInterface(Interface i){
	interface = i;
}

Datecs::Datecs(){
	memset(frame, ' ', sizeof(frame));
	interface = nullptr;
}

void Datecs::init(){
	const uint8_t bff[] = { 0x1B, 0x40 }; // Resets the various display settings to their initial values
	interface(bff, sizeof(bff));

	// Define user-defined characters degree Celsius
	const uint8_t setchar7D[] = {
		0x1B, 0x26, 0x7D,
		0b11100101,
		0b00111000,
		0b00000000,
		0b00000000,
		0b00000000
	};
	interface(setchar7D, sizeof(setchar7D));

	// Define user-defined characters CO2
	const uint8_t setchar7E[] = {
		0x1B, 0x26, 0x7E,
		0b11000100,
		0b00110000,
		0b00111100,
		0b11101011,
		0b01100000
	};
	interface(setchar7E, sizeof(setchar7E));

	// Select user defined character
	const uint8_t setuserchar[] = { 0x1B, 0x25, 0x01 };
	interface(setuserchar, sizeof(setuserchar));
}

void Datecs::brightness(uint8_t level){
	const uint8_t bff[] = { 0x1F, 0x58, level };
	interface(bff, sizeof(bff));
}

void Datecs::clear(){
	memset(frame, ' ', sizeof(frame));
}

void Datecs::putstring(uint8_t x, uint8_t y, const char* s){
	while(*s){
		frame[y][x] = *s++;
		x++;
		if(x > 19){
			break;
		}
	}
}

void Datecs::turnAnnunciator(bool on, int8_t x){
	const uint8_t bff[] = { 0x1F, 0x23, on ? uint8_t(1) : uint8_t(0), uint8_t(x + 1) };
	interface(bff, sizeof(bff));
}

void Datecs::flush(){
	const uint8_t bff[] = { 0x1B, 0x48, 0 }; // Set cursor to 0
	interface(bff, sizeof(bff));
	interface(frame, sizeof(frame));
}
