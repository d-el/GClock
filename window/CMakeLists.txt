add_library(window STATIC
	src/base.cpp
	src/font5x7.c
	#src/menuSystem.cpp
	#src/settingTSK.cpp
	#src/startupTSK.cpp
	)

target_include_directories(window PUBLIC
	include
	PRIVATE ../app/inc
	)

target_link_libraries(window freertos drivers dev utils prmsystem task)
