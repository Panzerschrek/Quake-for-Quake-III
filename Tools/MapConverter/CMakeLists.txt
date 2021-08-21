include(../cxx_common.cmake)

set(
	SOURCES
		main.cpp
	)
add_executable(MapConverter ${SOURCES})
target_link_libraries(MapConverter PRIVATE BspcLib)
