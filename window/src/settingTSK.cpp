/*!****************************************************************************
 * @file		settingTSK.c
 * @author		d_el - Storozhenko Roman
 * @version		V1.1
 * @date		01.01.2016
 * @copyright	The MIT License (MIT). Copyright (c) 2020 Storozhenko Roman
 * @brief		This task create start screen
 */

/*!****************************************************************************
 * Include
 */
#include <FreeRTOS.h>
#include <task.h>
#include <enco.h>
//#include <systemTSK.h>
#include "settingTSK.h"
#include "menuSystem.h"

using namespace Menu;

/*!****************************************************************************
 * @brief    set LCD Bright callback
 */
ItemState setBright(const MenuItem* m){
	(void)m;

	return ItemState{ true, "" };
}

/*!****************************************************************************
 * @brief    set LCD Bright callback
 */
ItemState updateDST(const MenuItem* m){
	(void)m;
	//timezoneUpdate(Prm::timezone.val);
	return ItemState { true, "" };
}

/*!****************************************************************************
 * @brief    RTC select callback
 */
ItemState rtcSelect(const MenuItem* m){
	(void)m;
	return ItemState { true, "" };
}

/*!****************************************************************************
 * @brief    RTC select callback
 */
ItemState rtcUnselect(const MenuItem* m){
	(void)m;
	return ItemState { true, "" };
}

/*!****************************************************************************
 * @brief    setup regulator
 */
ItemState PrepareU(const MenuItem* m){
}

/*!****************************************************************************
 * @brief    setup regulator
 */
ItemState PrepareI(const MenuItem* m){
}

/*!****************************************************************************
 * @brief
 */
ItemState updateReg(const MenuItem* m){
}

/*!****************************************************************************
 * @brief
 */
ItemState savePointU(const MenuItem* m){
}

/*!****************************************************************************
 * @brief
 */
ItemState savePointI(const MenuItem* m){
}

/*!****************************************************************************
 * @brief    NET pfUnselect
 */
ItemState netUpdate(const MenuItem* m){
}

/*!****************************************************************************
 * @brief
 */
ItemState calibrExit(const MenuItem* m){
}

extern const MenuItem
m1,
	m00,
	m01,
	m02,
	m03,
		m000,
		m001,
		m002,
		m003,
		m004,
	m10,
	m11,
	m12,
	m13,
		m100,
		m101,
		m102,
		m103,
		m104,
		m105,
m2,
	m20,
	m21,
	m22,
	m23,
		m230,
		m231,
		m232,
		m233,
		m234,
		m235,
	m24,
		m240,
		m241,
		m242,
		m243,
		m244,
		m245,
m3,
	m30,
	m31,
	m32,
m4;

const MenuItem
m0("Vmeter", nullptr, true, 0, nullptr, nullptr, nullptr, nullptr, &m1, nullptr, &m00),
	m00("Point1", nullptr, true, 0, nullptr, PrepareU, calibrExit, nullptr, &m01, nullptr, &m000),
	m01("Point2", nullptr, true, 1, nullptr, PrepareU, calibrExit, nullptr, &m02, &m00, &m000),
	m02("Point3", nullptr, true, 2, nullptr, PrepareU, calibrExit, nullptr, &m03, &m01, &m000),
	m03("Point4", nullptr, true, 3, nullptr, PrepareU, calibrExit, nullptr, nullptr, &m02, &m000),
		m000("Ureal", &Prm::int16, true, 0, savePointU, nullptr, nullptr, updateReg, &m001, nullptr, nullptr),
		m001("DacU", &Prm::int16, true, 0, savePointU, nullptr, nullptr, updateReg, &m002, &m000, nullptr),
		m002("AdcU", &Prm::int16, false, 0, savePointU, nullptr, nullptr, updateReg, &m003, &m001, nullptr),
		m003("AdcI", &Prm::int16, false, 0, savePointU, nullptr, nullptr, updateReg, &m004, &m002, nullptr),
		m004("Umeas", &Prm::int16, false, 0, savePointU, nullptr, nullptr, updateReg, nullptr, &m003, nullptr),

m1("Ameter", nullptr, true, 0, nullptr, nullptr, nullptr, nullptr, &m2, &m0, &m10),
	m10("Point1", nullptr, true, 0, nullptr, PrepareI, calibrExit, nullptr, &m11, nullptr, &m100),
	m11("Point2", nullptr, true, 1, nullptr, PrepareI, calibrExit, nullptr, &m12, &m10, &m100),
	m12("Point3", nullptr, true, 2, nullptr, PrepareI, calibrExit, nullptr, &m13, &m11, &m100),
	m13("Point4", nullptr, true, 3, nullptr, PrepareI, calibrExit, nullptr, nullptr, &m12, &m100),
		m100("Ireal", &Prm::int16, true, 0, savePointI, nullptr, nullptr, nullptr, &m101, nullptr, nullptr),
		m101("DacI", &Prm::int16, true, 0, savePointI, nullptr, nullptr, updateReg, &m102, &m100, nullptr),
		m102("AdcU", &Prm::int16, false, 0, savePointI, nullptr, nullptr, updateReg, &m103, &m101, nullptr),
		m103("AdcI", &Prm::int16, false, 0, savePointI, nullptr, nullptr, updateReg, &m104, &m102, nullptr),
		m104("AdcIEx", &Prm::int16, false, 0, savePointI, nullptr, nullptr, updateReg, &m105, &m103, nullptr),
		m105("Imeas", &Prm::int16, false, 0, savePointI, nullptr, nullptr, updateReg, nullptr, &m104, nullptr),

m2("Date&Time", nullptr, true, 0, nullptr, nullptr, nullptr, nullptr, &m3, &m1, &m20),
	m20("Clock", &Prm::int16, true, 0, nullptr, rtcSelect, rtcUnselect, nullptr, &m21, nullptr, nullptr, clockEditor),
	m21("Time Zone", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m22, &m20, nullptr),
	m22("DST", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m23, &m21, nullptr),
	m23("Start", nullptr, true, 0, nullptr, nullptr, updateDST, nullptr, &m24, &m22, &m230),
		m230("Mon", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m231, nullptr, nullptr),
		m231("Week", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m232, &m230, nullptr),
		m232("Day", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m233, &m231, nullptr),
		m233("Hour", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m234, &m232, nullptr),
		m234("Min", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m235, &m233, nullptr),
		m235("Sec", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, nullptr, &m234, nullptr),
	m24("End", nullptr, true, 0, nullptr, nullptr, updateDST, nullptr, nullptr, &m23, &m240),
		m240("Mon", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m241, nullptr, nullptr),
		m241("Week", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m242, &m240, nullptr),
		m242("Day", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m243, &m241, nullptr),
		m243("Hour", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m244, &m242, nullptr),
		m244("Min", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m245, &m243, nullptr),
		m245("Sec", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, nullptr, &m244, nullptr),

m3("LAN", nullptr, true, 0, nullptr, nullptr, netUpdate, nullptr, &m4, &m2, &m30),
	m30("IP address", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m31, nullptr, nullptr, ipAddressEditor),
	m31("subnet mask", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, &m32, &m30, nullptr, ipAddressEditor),
	m32("gateway", &Prm::int16, true, 0, nullptr, nullptr, nullptr, nullptr, nullptr, &m31, nullptr, ipAddressEditor),

m4("Bright", &Prm::int16, true, 0, setBright, nullptr, nullptr, nullptr, nullptr, &m3, nullptr);

/*!****************************************************************************
 * @brief    Setting system task
 */
void settingTSK(void *pPrm){
	(void)pPrm;
	run(&m0);
	//saveparametersSystem();
	//selWindow(baseWindow);
	vTaskSuspend(NULL);
}

/******************************** END OF FILE ********************************/
