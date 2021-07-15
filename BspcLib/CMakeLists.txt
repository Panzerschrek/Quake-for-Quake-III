set(Q3_CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Quake-III-Arena/code)
set(BSPC_DIR ${Q3_CODE_DIR}/bspc)
set(BOTLIB_DIR ${Q3_CODE_DIR}/botlib)
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Quake-III-Arena/common)

set(
	SOURCES
		${BSPC_DIR}/l_bsp_ent.c
		${BSPC_DIR}/l_bsp_ent.h
		${BSPC_DIR}/l_bsp_q1.c
		${BSPC_DIR}/l_bsp_q1.h
		${BSPC_DIR}/l_bsp_q3.c
		${BSPC_DIR}/l_bsp_q3.h
		${BSPC_DIR}/l_cmd.c
		${BSPC_DIR}/l_log.c
		${BSPC_DIR}/l_math.c
		${BSPC_DIR}/l_mem.c
		${BOTLIB_DIR}/l_script.c
		${BOTLIB_DIR}/l_script.h
		${COMMON_DIR}/imagelib.c
		BspcLibIncludes.hpp
		CommonFuncs.c
	)
add_library(BspcLib ${SOURCES})
target_include_directories(BspcLib PUBLIC ${BSPC_DIR} ${BOTLIB_DIR} ${COMMON_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(BspcLib PRIVATE QDECL= BSPC=)
