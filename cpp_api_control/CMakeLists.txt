cmake_minimum_required(VERSION 3.10) 
project(server_streaming) 

find_package(PkgConfig REQUIRED)
find_package(Crow CONFIG REQUIRED) 
find_package(OpenCV REQUIRED)

add_executable(STREAMING_SERVER src/main.cpp)

target_include_directories(STREAMING_SERVER PRIVATE headers)
target_link_libraries(STREAMING_SERVER PRIVATE Crow::Crow ${GSTREAMER_LIBRARIES} ${OpenCV_LIBS})