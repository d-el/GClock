cmake_minimum_required(VERSION 3.6)

PROJECT(GClock
	VERSION 0.2.0
	LANGUAGES CXX C
	)
	
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

add_compile_options(
	-DSTM32G070xx
	)

add_compile_options(
	-g3 -O2
	-fno-exceptions
	-fmessage-length=0
	-ffunction-sections
	-fdata-sections
	-fsigned-char
	-fsingle-precision-constant
	-Wfloat-equal
	-Wuninitialized
	-Wextra
	-Wall
	-fconcepts-ts
	-fno-common

	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
	)

add_link_options(
	-Wl,--flto
	-Wl,--gc-sections
	#-Wl,--print-gc-sections
	-Wl,--print-memory-usage
	-Wl,--undefined=uxTopUsedPriority
	-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
	)

configure_file(utils/include/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/utils/include/version.h @ONLY)

add_subdirectory(config)
add_subdirectory(dev)
add_subdirectory(drivers)
add_subdirectory(lib/bme280)
add_subdirectory(lib/CMSIS)
add_subdirectory(lib/freertos)
add_subdirectory(lib/minmea)
add_subdirectory(math)
add_subdirectory(prmsystem)
add_subdirectory(systems)
add_subdirectory(task)
add_subdirectory(utils)
add_subdirectory(window)

set(SOURCE_FILES
	main.cpp
	)

add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}.elf
	drivers
	freertos
	systems
	task
	utils
	)

set(NAME ${PROJECT_NAME}_${PROJECT_VERSION})
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
	BYPRODUCTS ${NAME}.hex
	COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${NAME}.hex

	BYPRODUCTS ${NAME}.bin
	COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${NAME}.bin

	COMMENT "Generate hex and bin"
	)
