set(Q3_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../Quake-III-Arena)

file(GLOB SOURCES ${Q3_SOURCES}/q3map/*.c)
file(GLOB SOURCES_EXCLUDE ${Q3_SOURCES}/q3map/gldraw.c)
list(REMOVE_ITEM SOURCES ${SOURCES_EXCLUDE})

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

add_executable(q3map ${SOURCES} ${SOURCES_COMMON} ${SOURCES_PLATFORM})
