add_library(dev STATIC
	src/datecs.cpp
	src/ds18b20.c
	src/oneWireUart.c
	)

target_include_directories(dev PUBLIC
	include
	)

target_link_libraries(dev drivers utils freertos)
