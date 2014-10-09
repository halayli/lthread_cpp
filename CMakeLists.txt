cmake_minimum_required (VERSION 2.6)
project (lthread_cpp)
SET(CMAKE_BUILD_TYPE release)
SET (CMAKE_C_FLAGS_RELEASE "-Werror -g -Wall")

include_directories(/usr/local/include include)
set(lthread_files src/socket.cc src/lthread.cc src/ssl.cc
    src/proxy.cc src/listener.cc src/shell.cc)

add_library(lthread_cpp ${lthread_files})
set_target_properties(lthread_cpp PROPERTIES COMPILE_FLAGS "-std=c++11")
target_link_libraries(lthread_cpp pthread lthread ssl crypto)
set_property(SOURCE ${lthread_files} PROPERTY COMPILE_FLAGS "-O2")

install(TARGETS lthread_cpp DESTINATION lib)
install(DIRECTORY include/lthread_cpp DESTINATION include)

set(UNIT_TEST_PATH ${CMAKE_SOURCE_DIR}/tests)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${UNIT_TEST_PATH})
include_directories(src)
