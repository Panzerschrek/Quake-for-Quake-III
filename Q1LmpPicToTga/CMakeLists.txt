include(../cxx_common.cmake)

add_executable(Q1LmpPicToTga main.cpp)
target_link_libraries(Q1LmpPicToTga PRIVATE BspcLib)
