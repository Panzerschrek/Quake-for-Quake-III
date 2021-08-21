include(../cxx_common.cmake)

add_executable(Q1PicToTga main.cpp)
target_link_libraries(Q1PicToTga PRIVATE BspcLib)
