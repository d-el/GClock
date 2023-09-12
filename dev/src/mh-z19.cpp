/*!****************************************************************************
 * @file		mh-z19.cpp
 * @author		d_el
 * @version		V1.0
 * @date		01.11.2022
 * @copyright	License (MIT). Copyright (c) 2022 Storozhenko Roman
 * @brief
 */

/*!****************************************************************************
 * Include
 */
#include <assert.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <uart.h>

/*!****************************************************************************
 * MEMORY
 */
static SemaphoreHandle_t txSem;
static SemaphoreHandle_t rxSem;

#define mhuart uart1

#define ReadCO2					0x86	// Read CO2 concentration
#define CalibrateZeroPoint		0x87	// Calibrate Zero Point (ZERO)
#define CalibrateSpanPoint		0x88	// Calibrate Span Point (SPAN)
#define ONOFFAutoCalibration	0x79	// ON/OFF Auto Calibration
#define DetectionRangeSetting	0x99	// Detection range setting

/*!****************************************************************************
 * @brief
 */
void mh_z19_init(void){
	vSemaphoreCreateBinary(rxSem);
	xSemaphoreTake(rxSem, portMAX_DELAY);
	assert(rxSem != NULL);

	// uart RX TX callback
	auto uartRxHook = [](uart_type *puart){
		(void)puart;
		BaseType_t xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(rxSem, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	};
	auto uartTxHook = [](uart_type *puart){
		(void)puart;
		BaseType_t xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(txSem, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	};

	uart_init(mhuart, 9600);
	uart_setCallback(mhuart, uartTxHook, uartRxHook);
}

static constexpr uint8_t getCheckSum(const uint8_t *packet, size_t len){
	uint8_t checksum = 0xFF;
	for(uint8_t i = 0; i < len; i++){
		checksum -= packet[i];
	}
	checksum += 1;
	return checksum;
}

static bool mh_z19_transaction(const uint8_t* tx, size_t txlen, uint8_t* rx, size_t rxlen){
	if(rxlen){
		uart_read(mhuart, rx, rxlen);
	}
	uart_write(mhuart, tx, txlen);
	BaseType_t res = xSemaphoreTake(rxlen ? rxSem : txSem, pdMS_TO_TICKS(1000));
	if(res != pdTRUE){
		return false;
	}
	return true;
}

int16_t mh_z19_readCO2(void){
	const uint8_t txbff[9] = { 0xFF, 0x01, ReadCO2, 0, 0, 0, 0, 0, getCheckSum(&txbff[1], 7) };
	uint8_t rxbff[9];
	if(mh_z19_transaction(txbff, sizeof(txbff), rxbff, sizeof(rxbff)) && rxbff[1] == ReadCO2 && getCheckSum(&rxbff[1], 8) == 0){
		return rxbff[2] << 8 | rxbff[3];
	}
	return -1;
}

bool mh_z19_zeroPointCalibration(){
	const uint8_t txbff[9] = { 0xFF, 0x01, CalibrateZeroPoint, 0, 0, 0, 0, 0, getCheckSum(&txbff[1], 7) };
	return mh_z19_transaction(txbff, sizeof(txbff), nullptr, 0);
}

bool mh_z19_spanPointCalibration(uint16_t span){
	const uint8_t txbff[9] = { 0xFF, 0x01, CalibrateSpanPoint, uint8_t(span >> 8), uint8_t(span | 0xFF), 0, 0, 0, getCheckSum(&txbff[1], 7) };
	return mh_z19_transaction(txbff, sizeof(txbff), nullptr, 0);
}

bool mh_z19_abcLogicOn(bool on){
	const uint8_t txbff[9] = { 0xFF, 0x01, ONOFFAutoCalibration, uint8_t(on ? 0xA0 : 0), 0, 0, 0, 0, getCheckSum(&txbff[1], 7) };
	return mh_z19_transaction(txbff, sizeof(txbff), nullptr, 0);
}

bool mh_z19_sensorRange(uint16_t range){
	const uint8_t txbff[9] = { 0xFF, 0x01, CalibrateSpanPoint, uint8_t(range >> 8), uint8_t(range | 0xFF), 0, 0, 0, getCheckSum(&txbff[1], 7) };
	return mh_z19_transaction(txbff, sizeof(txbff), nullptr, 0);
}

/******************************** END OF FILE ********************************/
