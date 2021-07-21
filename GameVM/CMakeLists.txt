if(WIN32)
	add_definitions(-D__WIN64__)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(LIB_SUFFIX "x86")
else()
	set(LIB_SUFFIX "x86_64")
endif()

file(GLOB QCOMMON_SOURCES code/qcommon/*)
add_library(qcommon STATIC ${QCOMMON_SOURCES})


file(GLOB SERVER_SOURCES code/game/*)
add_library(qagame${LIB_SUFFIX} SHARED ${SERVER_SOURCES})
target_link_libraries(qagame${LIB_SUFFIX} qcommon)


file(GLOB CLIENT_SOURCES
	code/cgame/*
	code/game/bg_misc.c
	code/game/bg_pmove.c
	code/game/bg_slidemove.c
	)
add_library(cgame${LIB_SUFFIX} SHARED ${CLIENT_SOURCES})
target_link_libraries(cgame${LIB_SUFFIX} qcommon)
