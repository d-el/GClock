/*!****************************************************************************
 * @file		sensorTSK.c
 * @author		d_el
 * @version		V1.0
 * @date		Jan 6, 2024
 * @copyright	License (MIT). Copyright (c) 2024 Storozhenko Roman
 * @brief
 */

/*!****************************************************************************
 * Include
 */
#include <assert.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <prmSystem.h>
#include <i2c.h>
#include <uart.h>
#include <hdc1080.h>
#include <bme280.h>
#include "sensorTSK.h"
#include <minmea/minmea.h>
#include <plog.h>
#include "rtc.h"
#include "timegm.h"

#define UART_BUF_RX		UART2_RxBffSz
#define UART			uart2

/*!****************************************************************************
 * MEMORY
 */
#define LOG_LOCAL_LEVEL P_LOG_INFO
static const char *logTag = "sensorTSK";
static SemaphoreHandle_t i2cTcSem;

/*!****************************************************************************
 * @brief
 */
bool i2cReadReg(uint8_t devaddr, uint8_t regaddr, void *dst, uint16_t len, uint16_t delay_ms){
	i2c_write(i2c2, &regaddr, 1, devaddr, i2cNeedStop);
	BaseType_t res = xSemaphoreTake(i2cTcSem, pdMS_TO_TICKS(100));
	if(res != pdTRUE){
		return false;
	}

	if(delay_ms){
		vTaskDelay(pdMS_TO_TICKS(delay_ms));
	}

	i2c_read(i2c2, dst, len, devaddr | 1);
	res = xSemaphoreTake(i2cTcSem, pdMS_TO_TICKS(100));
	if(res != pdTRUE){
		return false;
	}
	return true;
}

/*!****************************************************************************
 * @brief
 */
bool i2cWriteReg(uint8_t devddr, uint8_t regaddr, const void *src, uint16_t len){
	if(len > sizeof(i2c2TxBff) / 2){
		return false;
	}
	const uint8_t *srcbyte = (const uint8_t*)src;
	for(int i = 0; i < len; i++){
		i2c2TxBff[i * 2] = regaddr++;
		i2c2TxBff[i * 2 + 1] = *srcbyte++;
	}

	i2c_write(i2c2, &i2c2TxBff[0], len * 2, devddr, i2cNeedStop);
	BaseType_t res = xSemaphoreTake(i2cTcSem, pdMS_TO_TICKS(100));
	if(res != pdTRUE){
		return false;
	}
	return true;
}

/*!****************************************************************************
 * @brief
 */
static void i2cTxHook(i2c_type* i2c){
	(void)i2c;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(i2cTcSem, &xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
};

/*!****************************************************************************
 * @brief
 */
void sensorTSK(void *pPrm){
	(void)pPrm;
	i2c_init(i2c2);
	vSemaphoreCreateBinary(i2cTcSem);
	xSemaphoreTake(i2cTcSem, portMAX_DELAY);
	assert(i2cTcSem != NULL);
	i2c_setCallback(i2c2, i2cTxHook);

	uart_init(UART, 9600);
	uart_setCallback(UART, NULL, NULL);

	hdc1080dev_t hdc1080dev = {};
	hdc1080dev.regRead = i2cReadReg;
	hdc1080dev.regWrite = i2cWriteReg;
	if(!hdc1080_init(&hdc1080dev)){
		Prm::humidity.val = -1;
	}

	struct bme280_dev dev = {};
	dev.intf = BME280_I2C_INTF;
	dev.read = [] (uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) -> int8_t {
		(void)intf_ptr;
		int8_t r = i2cReadReg(BME280_I2C_ADDR_PRIM << 1, reg_addr, (void*)reg_data, len, 0) ? 0 : -1;
		while(r){
		}
		return r;
	};
	dev.write = [] (uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) -> int8_t {
		(void)intf_ptr;
		int8_t r = i2cWriteReg(BME280_I2C_ADDR_PRIM << 1, reg_addr, (const void*)reg_data, len) ? 0 : -1;
		while(r){
		}
		return r;
	};
	dev.delay_us = [] (uint32_t period, void *intf_ptr) -> void {
		(void)intf_ptr;
		vTaskDelay(pdMS_TO_TICKS(period / 1000 + 1));
	};
	int8_t rslt = bme280_init(&dev);
	if(rslt == BME280_OK){
		struct bme280_settings settings = {};
		/* Always read the current settings before writing, especially when all the configuration is not modified */
		rslt = bme280_get_sensor_settings(&settings, &dev);
		/* Configuring the over-sampling rate, filter coefficient and standby time */
		/* Overwrite the desired settings */
		settings.filter = BME280_FILTER_COEFF_16;
		/* Over-sampling rate for humidity, temperature and pressure */
		settings.osr_h = BME280_OVERSAMPLING_16X;
		settings.osr_p = BME280_OVERSAMPLING_16X;
		settings.osr_t = BME280_OVERSAMPLING_16X;
		/* Setting the standby time */
		settings.standby_time = BME280_STANDBY_TIME_20_MS;
		rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &dev);
		/* Always set the power mode after setting the configuration */
		rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev);
		/* Calculate measurement time in microseconds */
		uint32_t period;
		rslt = bme280_cal_meas_delay(&period, &settings);
	}

	uart_read(UART, UART->pRxBff, UART_BUF_RX);

	while(1){
		uint16_t rh;
		Prm::humidity.val = hdc1080_readRh(&hdc1080dev, &rh) ? rh : -1;

		uint8_t status_reg = 0;
		rslt = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, &dev);
		if (status_reg & BME280_STATUS_MEAS_DONE)
		{
			/* Measurement time delay given to read sample */
			//dev.delay_us(period, dev.intf_ptr);
			struct bme280_data comp_data = {};
			/* Read compensated data */
			rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
			Prm::bme280pressure.val = comp_data.pressure;
			Prm::bme280temperature.val = comp_data.temperature;
			Prm::bme280humidity.val = comp_data.humidity / 100;
		}

		// Read from GPS
		size_t numRx = UART_BUF_RX - uartGetRemainRx(UART);
		// Parse GPS
		if((numRx != 0)&&(UART->rxState == uartRxSuccess)){
			uart_read(UART, UART->pRxBff, UART_BUF_RX);

			// Parse NMEA
			const char *separator = "\n";
			char *lasts = nullptr;
			char *line = strtok_r((char *)UART->pRxBff, separator, &lasts);
			while(line != nullptr && line[0] == '$'){
				P_LOGD(logTag, "GPS %s", line);
				switch(minmea_sentence_id(line, false)){
					case MINMEA_SENTENCE_GGA: {
						struct minmea_sentence_gga frame;
						if(minmea_parse_gga(&frame, line)){
							Prm::satellites.val = frame.satellites_tracked;
							const int32_t newscale = 100000;
							int32_t nmealat = minmea_rescale(&frame.latitude, newscale);
							Prm::glatitude.val = (nmealat / (newscale * 100)) * newscale + (nmealat % (newscale * 100)) / 60;
							int32_t nmealon = minmea_rescale(&frame.longitude, newscale);
							Prm::glongitude.val = (nmealon / (newscale * 100)) * newscale + (nmealon % (newscale * 100)) / 60;
							Prm::ghdop.val = minmea_rescale(&frame.hdop, 100);
						}
					} break;

					case MINMEA_SENTENCE_RMC: {
						struct minmea_sentence_rmc frame;
						if(minmea_parse_rmc(&frame, line) && frame.valid){
							struct tm gpstm = {};
							gpstm.tm_year = frame.date.year + 100;
							gpstm.tm_mon = frame.date.month - 1;
							gpstm.tm_mday = frame.date.day;
							gpstm.tm_hour = frame.time.hours;
							gpstm.tm_min = frame.time.minutes;
							gpstm.tm_sec = frame.time.seconds;
							gpstm.tm_isdst = 0;
							time_t gpsUnixTime = timegm(&gpstm);
							if(time(NULL) != gpsUnixTime){
								rtc_setTimeUnix(gpsUnixTime);
							}
						}else{

						}
					} break;
					default: ;
				}
				line = strtok_r(NULL, separator, &lasts);
			}
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

/******************************** END OF FILE ********************************/
