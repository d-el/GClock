add_library(dev STATIC
	src/display.c
	src/ds18b20.c
	src/graphics.c
	src/hdc1080.c
	src/mh-z19.cpp
	src/oneWireUart.c
	src/vfd.c
	)

target_include_directories(dev PUBLIC
	include
	)

target_link_libraries(dev drivers utils freertos)
