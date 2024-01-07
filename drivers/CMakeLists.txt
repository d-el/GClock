add_library(drivers STATIC
	src/adc.c
	src/clock.c
	src/drivers.c
	src/enco.c
	src/flash.c
	src/gpio.c
	src/i2c.c
	src/key.c
	src/rtc.c
	src/spi.c
	src/uart.c
	)

target_include_directories(drivers PUBLIC
	include
	)

target_link_libraries(drivers cmsis)

target_link_options(drivers PUBLIC
	-uDMA1_Channel1_IRQHandler
	)