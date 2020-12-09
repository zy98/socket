cmake_minimum_required (VERSION 3.13)

project (echo)

#message(STATUS "Hello: ${PROJECT_SOURCE_DIR}")
aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE_DIR)

add_executable(echo ${SOURCE_DIR})

# 添加链接库
if(UNIX)
    target_link_libraries(echo pthread)
elseif(WIN32)
    target_link_libraries(echo ws2_32)
endif()