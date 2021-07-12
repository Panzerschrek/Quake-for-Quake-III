set(BSPC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Quake-III-Arena/code/bspc)

set(
	SOURCES
		${BSPC_DIR}/l_bsp_q1.c
		${BSPC_DIR}/l_bsp_q1.h
		${BSPC_DIR}/l_bsp_q3.c
		${BSPC_DIR}/l_bsp_q3.h
		BspcLibIncludes.hpp
	)
add_library(BspcLib ${SOURCES})
target_include_directories(BspcLib PUBLIC ${BSPC_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(BspcLib PRIVATE QDECL=)
