if(WIN32)
	add_definitions(-D__WIN64__)
else()
	add_definitions(-DARCH_STRING=x64)
	add_compile_options(-fPIC)
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	add_compile_options(-fvisibility=hidden -fvisibility-inlines-hidden)
	list(APPEND CMAKE_SHARED_LINKER_FLAGS_RELEASE "-s -Wl,--gc-sections")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(LIB_SUFFIX "x86")
else()
	set(LIB_SUFFIX "x86_64")
endif()

# QCommon library
file(GLOB QCOMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/code/qcommon/*.c)
file(GLOB QCOMMON_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/code/qcommon/*.h)
add_library(qcommon STATIC ${QCOMMON_SOURCES} ${QCOMMON_HEADERS})

# Server
file(GLOB SERVER_SOURCES code/game/*.c)
file(GLOB SERVER_SOURCES_MAIN code/game/g_main.c)
list(REMOVE_ITEM SERVER_SOURCES ${SERVER_SOURCES_MAIN})
file(GLOB SERVER_HEADERS code/game/*.h)
file(GLOB SERVER_SOURCES_NATIVE code/game/g_syscalls.c)
file(GLOB SERVER_SOURCES_VM_ASM code/game/g_syscalls.asm)
add_library(qagame${LIB_SUFFIX} SHARED ${SERVER_SOURCES} ${SERVER_SOURCES_MAIN} ${SERVER_HEADERS})
target_link_libraries(qagame${LIB_SUFFIX} qcommon)

# Client
file(GLOB CLIENT_SOURCES
	code/cgame/*.c
	code/game/bg_lib.c
	code/game/view.c
	)
file(GLOB CLIENT_SOURCES_MAIN code/cgame/cg_main.c)
list(REMOVE_ITEM CLIENT_SOURCES ${CLIENT_SOURCES_MAIN})
file(GLOB CLIENT_HEADERS code/cgame/*.h)
file(GLOB CLIENT_SOURCES_NATIVE code/cgame/cg_syscalls.c)
file(GLOB CLIENT_SOURCES_VM_ASM code/cgame/cg_syscalls.asm)
add_library(cgame${LIB_SUFFIX} SHARED ${CLIENT_SOURCES} ${CLIENT_SOURCES_MAIN} ${CLIENT_HEADERS})
target_link_libraries(cgame${LIB_SUFFIX} qcommon)

set(Q3_LCC q3lcc)
set(Q3_ASM q3asm)

#
# It's important to keep main.asm file first in list.
# I do not know why, but q3asm produces invalid QVM file input file with entry point is not first in the list.
#

# Server VM
list(APPEND SERVER_SOURCES_VM ${SERVER_SOURCES_MAIN})
list(APPEND SERVER_SOURCES_VM ${SERVER_SOURCES})
list(REMOVE_ITEM SERVER_SOURCES_VM ${SERVER_SOURCES_NATIVE})
list(APPEND SERVER_SOURCES_VM ${QCOMMON_SOURCES})
foreach(SOURCE ${SERVER_SOURCES_VM})
	file(RELATIVE_PATH SOURCE_REL ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCE})
	set(OUT_ASM_FILE ${CMAKE_CURRENT_BINARY_DIR}/game/${SOURCE_REL}.asm)
	add_custom_command(
		DEPENDS ${SOURCE}
		OUTPUT ${OUT_ASM_FILE}
		COMMAND ${Q3_LCC} ${SOURCE} -o ${OUT_ASM_FILE}
		)
	list(APPEND SERVER_LCC_ASM_FILES ${OUT_ASM_FILE})
endforeach()

add_custom_target(
	qagame.qvm ALL
	DEPENDS ${SERVER_LCC_ASM_FILES} ${SERVER_SOURCES_VM_ASM}
	COMMAND ${Q3_ASM} -o ${CMAKE_CURRENT_BINARY_DIR}/qagame.qvm ${SERVER_LCC_ASM_FILES} ${SERVER_SOURCES_VM_ASM}
	)

# Client VM
list(APPEND CLIENT_SOURCES_VM ${CLIENT_SOURCES_MAIN})
list(APPEND CLIENT_SOURCES_VM ${CLIENT_SOURCES})
list(REMOVE_ITEM CLIENT_SOURCES_VM ${CLIENT_SOURCES_NATIVE})
list(APPEND CLIENT_SOURCES_VM ${QCOMMON_SOURCES})
foreach(SOURCE ${CLIENT_SOURCES_VM})
	file(RELATIVE_PATH SOURCE_REL ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCE})
	set(OUT_ASM_FILE ${CMAKE_CURRENT_BINARY_DIR}/cgame/${SOURCE_REL}.asm)
	add_custom_command(
		DEPENDS ${SOURCE}
		OUTPUT ${OUT_ASM_FILE}
		COMMAND ${Q3_LCC} ${SOURCE} -o ${OUT_ASM_FILE}
		)
	list(APPEND CLIENT_LCC_ASM_FILES ${OUT_ASM_FILE})
endforeach()

add_custom_target(
	cgame.qvm ALL
	DEPENDS ${CLIENT_LCC_ASM_FILES} ${CLIENT_SOURCES_VM_ASM}
	COMMAND ${Q3_ASM} -o ${CMAKE_CURRENT_BINARY_DIR}/cgame.qvm ${CLIENT_LCC_ASM_FILES} ${CLIENT_SOURCES_VM_ASM}
	)
