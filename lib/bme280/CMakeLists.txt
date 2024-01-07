add_library(bme280 STATIC
	./bme280.c
	)

target_include_directories(bme280 PUBLIC
	./
	)

target_compile_options(bme280 PUBLIC -DBME280_32BIT_ENABLE=1)
