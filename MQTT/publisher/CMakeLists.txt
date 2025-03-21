cmake_minimum_required(VERSION 2.8 FATAL_ERROR) 
project(mqtt_pub) 

set(BUILD_DESTINATION, ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -Wall -fmessage-length=0") 
set(CMAKE_C_FLAGS_RELEASE "-O0 -Wall -fmessage-length=0") 

set(MQTT_LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../lib) 

set(SOURCE_LIST publisher.c) 

add_executable(mqtt_pub ${SOURCE_LIST})
target_link_libraries(mqtt_pub mosquitto) 

install(DIRECTORY DESTINATION ${BUILD_DESTINATION}/bin) 
install(TARGETS mqtt_pub RUNTIME DESTINATION ${BUILD_DESTINATION}/bin) 
