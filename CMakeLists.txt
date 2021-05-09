cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(PROJECT_NAME JKAProto)

set(SRC_DIR "src")
set(HDR_DIR "include")

file(GLOB SRC_FILES
    ${SRC_DIR}/*.cpp
    ${SRC_DIR}/*.h

    ${SRC_DIR}/jka/*.cpp
    ${SRC_DIR}/jka/*.h
    
    ${SRC_DIR}/packets/*.cpp
    ${SRC_DIR}/packets/*.h
)

file(GLOB HDR_FILES
    ${HDR_DIR}/JKAProto/*.h
    ${HDR_DIR}/JKAProto/jka/*.h
    ${HDR_DIR}/JKAProto/packets/*.h
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
           
project(${PROJECT_NAME} CXX)
include_directories(${HDR_DIR})
add_library(${PROJECT_NAME} STATIC
   ${SRC_FILES}
   ${HDR_FILES}
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-g>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:-O2>)