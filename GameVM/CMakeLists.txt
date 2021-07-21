if(WIN32)
	add_definitions(-D__WIN64__)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(LIB_SUFFIX "x86")
else()
	set(LIB_SUFFIX "x86_64")
endif()

file(GLOB QCOMMON_SOURCES code/qcommon/q_math.c code/qcommon/q_shared.c)
add_library(qcommon STATIC ${QCOMMON_SOURCES})


file(GLOB SERVER_SOURCES code/game/*)
file(GLOB SERVER_SOURCES_EXCLUDE code/game/g_rankings.c)
list(REMOVE_ITEM SERVER_SOURCES ${SERVER_SOURCES_EXCLUDE})
add_library(qagame${LIB_SUFFIX} SHARED ${SERVER_SOURCES})
target_link_libraries(qagame${LIB_SUFFIX} qcommon)


file(GLOB CLIENT_SOURCES
	code/cgame/*
	code/game/bg_misc.c
	code/game/bg_pmove.c
	code/game/bg_slidemove.c
	)
file(GLOB CLIENT_SOURCES_EXCLUDE code/cgame/cg_newdraw.c)
list(REMOVE_ITEM CLIENT_SOURCES ${CLIENT_SOURCES_EXCLUDE})
add_library(cgame${LIB_SUFFIX} SHARED ${CLIENT_SOURCES})
target_link_libraries(cgame${LIB_SUFFIX} qcommon)
