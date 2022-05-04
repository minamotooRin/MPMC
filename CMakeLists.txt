cmake_minimum_required(VERSION 3.4.1)
project(MPMC)

# SET(DEBUG_MODE TRUE)

if(DEBUG_MODE)
    message(STATUS "===Compiling in DEBUG mode...===" )
    add_definitions(-DDEBUG)
    add_definitions("-Wall -std=c++17 -O0 -g") # 编译选项
else()
    message(STATUS "===Compiling in Release mode...===" )
    add_definitions("-Wall -std=c++17 -O3") # 编译选项
endif()

# 打印信息
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)
add_library(MPMC SHARED
    ${SRC_LIST}
)

IF (WIN32)
	MESSAGE(STATUS "Now is windows")
ELSEIF (APPLE)
	MESSAGE(STATUS "Now is Apple systens.")
ELSEIF (UNIX)
	MESSAGE(STATUS "Now is UNIX-like OS's.")
    target_link_libraries(MPMC pthread)
ENDIF ()