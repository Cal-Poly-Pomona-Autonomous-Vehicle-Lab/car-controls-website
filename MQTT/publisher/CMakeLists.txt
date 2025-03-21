cmake_minimum_required(VERSION 2.8 FATAL_ERROR) 
project(publisher) 


set(BUILD_DESTINATION ${CMAKE_CURRENT_DIR}/..) 
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -Wall -fmessage-length=0") 
set(CMAKE_C_FLAGS_RELEASE "-O0 -Wall -fmessage-length=0") 

set(SOURCE_LIST 
    publisher.c) 

add_executable(publisher ${SOURCE_LIST}) 
target_link_libraries(publisher mosquitto)

install(DIRECTORY DESTINATION ${BUILD_DESTINATION}/bin) 
install(TARGETS publisher RUNTIME DESTINATION ${BUILD_DESTINATION}/bin) 
