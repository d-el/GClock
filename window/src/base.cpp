/*!****************************************************************************
 * @file		base.cpp
 * @author		d_el
 * @version		V1.0
 * @date		01.11.2022
 * @copyright	License (MIT). Copyright (c) 2022 Storozhenko Roman
 * @brief
 */

/*!****************************************************************************
 * Include
 */
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <display.h>
#include <graphics.h>
#include <enco.h>
#include <prmSystem.h>
#include <version.h>
#include <key.h>

//using display = Datecs;
constexpr uint8_t guiPeriod_ms = 15;

/*!****************************************************************************
 * MEMORY
 */

int16_t central();
int16_t minmaxOuter();
int16_t minmaxHome();
int16_t co2();
int16_t gps();
int16_t bme280();

/*!****************************************************************************
 * @brief
 */
void baseTSK(void* pPrm){
	(void)pPrm;

	// Startup animation
	for(int i = 0; i < 50; i++){
		for(int n = 0; n < 100; n++){
			uint8_t val = rand() % 14;
			grf_fillRect(n, 14 - val, 1, val, 1);
		}
		disp_flushfill(0);
		vTaskDelay(10);
	}

	disp_putStr(20, 0, &font5x7, 0, "DEL 2022-2024");
	disp_putStr(35, 7, &font5x7, 0, getVersion());
	disp_flushfill(0);
	vTaskDelay(pdMS_TO_TICKS(700));

	int8_t win = 0;
	while(1){
		switch(win){
			case -2:
				win += minmaxHome(); break;
			case -1:
				win += minmaxOuter(); break;
			case 0:
				win += central(); break;
//			case 1:
//				win += co2(); break;
			case 1:
				win += gps(); break;
			case 2:
				win += bme280(); break;
		}
		if(win < -2){
			win = -2;
		}
		if(win > 2){
			win = 2;
		}
	}
}

void setbr(){
	// Set display brightness
	uint8_t displayBrightness = 4;
	if(Prm::light < 100){
		displayBrightness = 100;
	}else if(Prm::light  < 300){
		displayBrightness = 50;
	}else if(Prm::light < 2000){
		displayBrightness = 25;
	}else if(Prm::light < 3000){
		displayBrightness = 20;
	}else{
		displayBrightness = 15;
	}
	vfd_brightness(99, displayBrightness);
}

int16_t central(){
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1){
		char s[32];
		time_t gpsUnixTime = Prm::gtime.val;
		if(gpsUnixTime){
			struct tm tm;
			localtime_r(&gpsUnixTime, &tm);
			snprintf(s, sizeof(s), "%02i:%02i:%02i", tm.tm_hour, tm.tm_min, tm.tm_sec);
			disp_putStr(0, 0, &font5x7, 0, s);
			snprintf(s, sizeof(s), "%02i:%02i:%02i", tm.tm_mday, tm.tm_mon + 1, tm.tm_year - 100);
			disp_putStr(0, 7, &font5x7, 0, s);
		}else{
			disp_putStr(0, 0, &font5x7, 0, "--:--:--");
			disp_putStr(0, 7, &font5x7, 0, "--.--.--");
		}

		size_t pressurelen = snprintf(s, sizeof(s), "%" PRIi32, (Prm::bme280pressure.val * 7500 + 500000) / 1000000);
		disp_putStr(19*5 - pressurelen*5, 7, &font5x7, 0, s);

		char humidity[8];
		Prm::humidity.tostring(humidity, sizeof(humidity));
		snprintf(s, sizeof(s), "%s%s", humidity, Prm::humidity.getunit());
		disp_putStr(15*5, 0, &font5x7, 0, s);

		// Show outer temperature
		char temperature[8];
		if(Prm::temp_out_ok){
			Prm::temp_out.tostring(temperature, sizeof(temperature));
			snprintf(s, sizeof(s), "%s\xF8", temperature);
		}else{
			snprintf(s, sizeof(s), "---\xF8");
		}
		disp_putStr(9*5, 0, &font5x7, 0, s);

		// Show in temperature
		if(Prm::temp_in_ok){
			Prm::temp_in.tostring(temperature, sizeof(temperature));
			snprintf(s, sizeof(s), "%s\xF8", temperature);
		}else{
			snprintf(s, sizeof(s), "---\xF8");
		}
		disp_putStr(9*5, 7, &font5x7, 0, s);

		setbr();

		int16_t tick = enco_read();
		if(tick){
			return tick;
		}

		disp_flushfill(0);

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(guiPeriod_ms));
	}
}

void makeminmaxstring(char* s, size_t size, auto &temperature, time_t time){
	if(abs(temperature.val) == Prm::mask_temp_minmax_noinit::minmax_noinit){
		snprintf(s, size, " ---.-\xF8 --.-- --:--");
	}else{
		char strtemperature[8];
		temperature.tostring(strtemperature, sizeof(strtemperature));
		int len = snprintf(s, size, "%s\xF8 ", strtemperature);
		struct tm tm;
		localtime_r(&time, &tm);
		strftime(&s[len], size - len, "%H:%M %d.%m", &tm);
	}
}

int16_t minmaxOuter(){
	vfd_blinker(9, true);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1){
		if(keyProc()){
			Prm::temp_out_min.setdef();
			Prm::temp_out_max.setdef();
		}

		disp_putStr(0, 0, &font5x7, 0, "O");
		disp_putStr(0, 7, &font5x7, 0, "u");

		char s[64];
		makeminmaxstring(s, sizeof(s), Prm::temp_out_max, Prm::temp_out_max_time.val);
		disp_putStr(2*5, 0, &font5x7, 0, s);
		makeminmaxstring(s, sizeof(s), Prm::temp_out_min, Prm::temp_out_min_time.val);
		disp_putStr(2*5, 7, &font5x7, 0, s);

		disp_flushfill(0);

		int16_t tick = enco_read();
		if(tick){
			vfd_blinker(9, false);
			return tick;
		}

		setbr();

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(guiPeriod_ms));
	}
}

int16_t minmaxHome(){
	vfd_blinker(8, true);
	TickType_t xLastWakeTime = xTaskGetTickCount();

	while(1){
		if(keyProc()){
			Prm::temp_in_min.setdef();
			Prm::temp_in_max.setdef();
		}

		disp_putStr(0, 0, &font5x7, 0, "I");
		disp_putStr(0, 7, &font5x7, 0, "n");

		char s[64];
		makeminmaxstring(s, sizeof(s), Prm::temp_in_max, Prm::temp_in_max_time.val);
		disp_putStr(2*5, 0, &font5x7, 0, s);

		makeminmaxstring(s, sizeof(s), Prm::temp_in_min, Prm::temp_in_min_time.val);
		disp_putStr(2*5, 7, &font5x7, 0, s);

		disp_flushfill(0);

		int16_t tick = enco_read();
		if(tick){
			vfd_blinker(8, false);
			return tick;
		}

		setbr();

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(guiPeriod_ms));
	}
}

int16_t co2(){
	vfd_blinker(11, true);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1){
		if(keyProc()){
			Prm::co2setZero.val = 1;
		}

		char s[32];
		snprintf(s, sizeof(s), "CO2 %i", Prm::co2.val);
		disp_putStr(1*5, 0, &font5x7, 0, s);

		disp_flushfill(0);

		int16_t tick = enco_read();
		if(tick){
			vfd_blinker(11, false);
			return tick;
		}

		setbr();

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(guiPeriod_ms));
	}
}

int16_t gps(){
	vfd_blinker(12, true);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1){
		char s[32];
		char sval[16];
		Prm::glatitude.tostring(sval, sizeof(sval));
		snprintf(s, sizeof(s), "L%s\xF8", sval);
		disp_putStr(0, 0, &font5x7, 0, s);

		Prm::glongitude.tostring(sval, sizeof(sval));
		snprintf(s, sizeof(s), "F%s\xF8", sval);
		disp_putStr(0, 7, &font5x7, 0, s);

		Prm::ghdop.tostring(sval, sizeof(sval));
		snprintf(s, sizeof(s), "hdop");
		disp_putStr(11*5, 0, &font5x7, 0, s);
		snprintf(s, sizeof(s), "%s%s", sval, Prm::ghdop.getunit());
		disp_putStr(11*5, 7, &font5x7, 0, s);

		Prm::satellites.tostring(sval, sizeof(sval));
		snprintf(s, sizeof(s), "s%s", sval);
		disp_putStr(17*5, 0, &font5x7, 0, s);

		disp_flushfill(0);

		int16_t tick = enco_read();
		if(tick){
			vfd_blinker(12, false);
			return tick;
		}

		setbr();

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(guiPeriod_ms));
	}
}

int16_t bme280(){
	vfd_blinker(13, true);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1){
		char s[32];
		char sval[16];
		Prm::bme280pressure.tostring(sval, sizeof(sval));
		snprintf(s, sizeof(s), "%s%s", sval, Prm::bme280pressure.getunit());
		disp_putStr(0, 0, &font5x7, 0, s);

		Prm::bme280temperature.tostring(sval, sizeof(sval));
		snprintf(s, sizeof(s), "%s%s", sval, Prm::bme280temperature.getunit());
		disp_putStr(0, 7, &font5x7, 0, s);

		Prm::bme280humidity.tostring(sval, sizeof(sval));
		snprintf(s, sizeof(s), "%s%s", sval, Prm::bme280humidity.getunit());
		disp_putStr(13*5, 0, &font5x7, 0, s);

		disp_flushfill(0);

		int16_t tick = enco_read();
		if(tick){
			vfd_blinker(13, false);
			return tick;
		}

		setbr();

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(guiPeriod_ms));
	}
}

/******************************** END OF FILE ********************************/
