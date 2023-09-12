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
#include <datecs.h>
#include <ds18TSK.h>
#include <enco.h>
#include <prmSystem.h>
#include <version.h>
#include <key.h>

using display = Datecs;
constexpr uint8_t guiPeriod_ms = 15;

/*!****************************************************************************
 * MEMORY
 */

int16_t central();
int16_t minmaxOuter();
int16_t minmaxHome();
int16_t co2();

/*!****************************************************************************
 * @brief
 */
void baseTSK(void* pPrm){
	(void)pPrm;

	// Startup animation
	for(size_t i = 0; i < 20; i++){
		display::get().clear();
		display::get().putstring(i, 0, ">");
		display::get().putstring(19 - i, 1, "<");
		display::get().flush();
		vTaskDelay(pdMS_TO_TICKS(15));
	}
	display::get().clear();
	display::get().putstring(6, 0, "DEL 2022");
	display::get().putstring(7, 1, getVersion());
	display::get().flush();
	vTaskDelay(pdMS_TO_TICKS(500));

	int8_t win = 0;
	while(1){
		switch(win){
			case -2:
				win += minmaxHome(); break;
			case -1:
				win += minmaxOuter(); break;
			case 0:
				win += central(); break;
			case 1:
				win += co2(); break;
		}
		if(win < -2){
			win = -2;
		}
		if(win > 1){
			win = 1;
		}
	}
}

int16_t central(){
	display::get().clear();
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1){
		char s[32];
		time_t gpsUnixTime = Prm::gtime.val;
		if(gpsUnixTime){
			struct tm tm;
			localtime_r(&gpsUnixTime, &tm);
			snprintf(s, sizeof(s), "%02i:%02i:%02i", tm.tm_hour, tm.tm_min, tm.tm_sec);
			Datecs::get().putstring(0, 0, s);
			snprintf(s, sizeof(s), "%02i:%02i:%02i", tm.tm_mday, tm.tm_mon + 1, tm.tm_year - 100);
			Datecs::get().putstring(0, 1, s);
		}else{
			Datecs::get().putstring(0, 0, "--:--:--");
			Datecs::get().putstring(0, 1, "--.--.--");
		}

		snprintf(s, sizeof(s), "S%i", Prm::satellites.val);
		Datecs::get().putstring(17, 0, s);

		// Show outer temperature
		if(Prm::temp_out_ok){
			snprintf(s, sizeof(s), "%+" PRIi16 ".%" PRIu16 "\x7D", Prm::temp_out.val / 10, abs(Prm::temp_out.val) % 10);
		}else{
			snprintf(s, sizeof(s), "---\x7D");
		}
		display::get().putstring(9, 0, s);

		// Show in temperature
		if(Prm::temp_in_ok){
			snprintf(s, sizeof(s), "%+" PRIi16 ".%" PRIu16 "\x7D", Prm::temp_in.val / 10, abs(Prm::temp_in.val) % 10);
		}else{
			snprintf(s, sizeof(s), "---\x7D");
		}
		display::get().putstring(9, 1, s);

		snprintf(s, sizeof(s), "\x7E" "%i", Prm::co2.val);
		Datecs::get().putstring(20 - strlen(s), 1, s);

		// Set display brightness
		uint8_t displayBrightness = 4;
		if(Prm::light < 100){
			displayBrightness = 4;
		}else if(Prm::light  < 300){
			displayBrightness = 3;
		}else if(Prm::light < 2000){
			displayBrightness = 2;
		}else{
			displayBrightness = 1;
		}
		Datecs::get().brightness(displayBrightness);

		int16_t tick = enco_read();
		if(tick){
			return tick;
		}

		display::get().flush();
		display::get().clear();
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(guiPeriod_ms));
	}
}

int16_t minmaxOuter(){
	display::get().clear();
	display::get().turnAnnunciator(true, 9);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1){
		if(keyProc()){
			Prm::temp_out_min.setdef();
			Prm::temp_out_max.setdef();
		}

		time_t time = Prm::temp_out_min_time.val;
		struct tm tm;
		localtime_r(&time, &tm);
		char stemperature[16];
		if(Prm::temp_out_min.val == Prm::mask_temp_minmax_noinit::minmax_noinit){
			snprintf(stemperature, sizeof(stemperature), "--- \x7D");
		}else{
			snprintf(stemperature, sizeof(stemperature), "%+" PRIi16 ".%" PRIu16 "\x7D", Prm::temp_out_min.val / 10, abs(Prm::temp_out_min.val) % 10);
		}
		char s[32];
		snprintf(s, sizeof(s), "Out min %s %02i:%02i", stemperature, tm.tm_hour, tm.tm_min);
		display::get().putstring(0, 0, s);

		time = Prm::temp_out_max_time.val;
		localtime_r(&time, &tm);
		if(Prm::temp_out_max.val == -Prm::mask_temp_minmax_noinit::minmax_noinit){
			snprintf(stemperature, sizeof(stemperature), "--- \x7D");
		}else{
			snprintf(stemperature, sizeof(stemperature), "%+" PRIi16 ".%" PRIu16 "\x7D", Prm::temp_out_max.val / 10, abs(Prm::temp_out_max.val) % 10);
		}
		snprintf(s, sizeof(s), "    max %s %02i:%02i", stemperature, tm.tm_hour, tm.tm_min);
		display::get().putstring(0, 1, s);

		int16_t tick = enco_read();
		if(tick){
			display::get().turnAnnunciator(false, 9);
			return tick;
		}

		display::get().flush();
		display::get().clear();
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(guiPeriod_ms));
	}
}

int16_t minmaxHome(){
	display::get().clear();
	display::get().turnAnnunciator(true, 8);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1){
		if(keyProc()){
			Prm::temp_in_min.setdef();
			Prm::temp_in_max.setdef();
		}

		time_t time = Prm::temp_out_min_time.val;
		struct tm tm;
		localtime_r(&time, &tm);
		char stemperature[16];
		if(Prm::temp_in_min.val == Prm::mask_temp_minmax_noinit::minmax_noinit){
			snprintf(stemperature, sizeof(stemperature), "--- \x7D");
		}else{
			snprintf(stemperature, sizeof(stemperature), "%+" PRIi16 ".%" PRIu16 "\x7D", Prm::temp_in_min.val / 10, abs(Prm::temp_in_min.val) % 10);
		}
		char s[32];
		snprintf(s, sizeof(s), "In  min %s %02i:%02i", stemperature, tm.tm_hour, tm.tm_min);
		display::get().putstring(0, 0, s);

		time = Prm::temp_in_max_time.val;
		localtime_r(&time, &tm);
		if(Prm::temp_in_max.val == -Prm::mask_temp_minmax_noinit::minmax_noinit){
			snprintf(stemperature, sizeof(stemperature), "--- \x7D");
		}else{
			snprintf(stemperature, sizeof(stemperature), "%+" PRIi16 ".%" PRIu16 "\x7D", Prm::temp_in_max.val / 10, abs(Prm::temp_in_max.val) % 10);
		}
		snprintf(s, sizeof(s), "    max %s %02i:%02i", stemperature, tm.tm_hour, tm.tm_min);
		display::get().putstring(0, 1, s);

		int16_t tick = enco_read();
		if(tick){
			display::get().turnAnnunciator(false, 8);
			return tick;
		}

		display::get().flush();
		display::get().clear();
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(guiPeriod_ms));
	}
}

int16_t co2(){
	display::get().clear();
	display::get().turnAnnunciator(true, 11);
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1){
		if(keyProc()){
			Prm::co2setZero.val = 1;
		}

		char s[32];
		snprintf(s, sizeof(s), "\x7E" "%i", Prm::co2.val);
		Datecs::get().putstring(1, 0, s);

		int16_t tick = enco_read();
		if(tick){
			display::get().turnAnnunciator(false, 11);
			return tick;
		}

		display::get().flush();
		display::get().clear();
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(guiPeriod_ms));
	}
}

/******************************** END OF FILE ********************************/
