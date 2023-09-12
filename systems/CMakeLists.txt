add_library(systems STATIC
	src/debugCore.c
	src/memmgr.c
	src/SEGGER_RTT.c
	src/semihosting.c
	src/startup_stm32g070xx.c
	src/syscall.c
	src/write.c
	)

target_include_directories(systems PUBLIC
	include
	)

target_link_libraries(systems cmsis drivers freertos utils)

target_compile_options(systems PUBLIC
	-DSEGGER_RTT_SECTION=".rtti"
	)

target_link_options(systems PUBLIC
	-Wl,--undefined=writeInit
	-Wl,--undefined=HardFault_Handler
	-Wl,--undefined=__wrap__malloc_r
	-Wl,--wrap,_malloc
	-Wl,--wrap,_free
	-Wl,--wrap,_calloc
	-Wl,--wrap,_realloc
	-Wl,--wrap,_malloc_r
	-Wl,--wrap,_free_r
	-Wl,--wrap,_calloc_r
	-Wl,--wrap,_realloc_r
	)
