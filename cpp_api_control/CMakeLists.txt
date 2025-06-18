cmake_minimum_required(VERSION 3.10) 
project(server_streaming) 

find_package(PkgConfig REQUIRED)
find_package(Crow CONFIG REQUIRED) 

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-base-1.0 gstreamer-app-1.0) 

include_directories(${GSTREAMER_INCLUDE_DIRS}) 

add_executable(STREAMING_SERVER src/main.cpp src/server.cpp)

target_include_directories(STREAMING_SERVER PRIVATE headers)
target_link_libraries(STREAMING_SERVER PRIVATE Crow::Crow ${GSTREAMER_LIBRARIES})

