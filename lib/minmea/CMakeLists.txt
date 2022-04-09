add_library(minmea STATIC
	minmea/minmea.c
	)

target_include_directories(minmea PUBLIC
	./
	)
