set(Q3_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../Quake-III-Arena)

file(GLOB SOURCES ${Q3_SOURCES}/q3map/*.c)
file(GLOB SOURCE_BUGGY_CAST ${Q3_SOURCES}/q3map/light_trace.c)
file(GLOB SOURCES_EXCLUDE ${Q3_SOURCES}/q3map/gldraw.c)
list(REMOVE_ITEM SOURCES ${SOURCES_EXCLUDE} ${SOURCE_BUGGY_CAST})

# Hack! Patch original code to fix fatal bug on x64 platform. I am too lasy to create own copy of Quake-III-arena repo.
set(SOURCE_BUGGY_CAST_PATCHED ${CMAKE_CURRENT_BINARY_DIR}/light_trace.c)
file(READ ${SOURCE_BUGGY_CAST} SOURCE_BUGGY_CAST_CONTENT)
string(REPLACE "tnodes = (tnode_t *)(((int)tnodes + 31)&~31);" "" SOURCE_BUGGY_CAST_CONTENT "${SOURCE_BUGGY_CAST_CONTENT}")
file(WRITE ${SOURCE_BUGGY_CAST_PATCHED} "${SOURCE_BUGGY_CAST_CONTENT}")

set(Q3_COMMON ${Q3_SOURCES}/common)
set(
	SOURCES_COMMON
		${Q3_COMMON}/aselib.c
		${Q3_COMMON}/bspfile.c
		${Q3_COMMON}/cmdlib.c
		${Q3_COMMON}/imagelib.c
		${Q3_COMMON}/mathlib.c
		${Q3_COMMON}/mutex.c
		${Q3_COMMON}/polylib.c
		${Q3_COMMON}/scriplib.c
		${Q3_COMMON}/threads.c
	)

include_directories(${Q3_SOURCES}/common)
include_directories(${Q3_SOURCES}/libs)
include_directories(${Q3_SOURCES}/q3map)

if(WIN32)
	file(GLOB SOURCES_PAK ${Q3_SOURCES}/libs/pak/*.cpp)
	file(GLOB SOURCES_JPEG ${Q3_SOURCES}/libs/jpeg6/*.cpp)
	set(SOURCES_PLATFORM ${SOURCES_PAK} ${SOURCES_JPEG})
else()
	add_definitions(-DCLK_TCK=1000)
	add_definitions(-DATOM=int)
	add_definitions(-Dboolean=int)
	add_definitions(-Dstrlwr=Q_strlwr)
	set(SOURCES_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/stuff.c)

	link_libraries(-lm)
endif()

add_executable(q3map ${SOURCES} ${SOURCES_COMMON} ${SOURCES_PLATFORM} ${SOURCE_BUGGY_CAST_PATCHED})
