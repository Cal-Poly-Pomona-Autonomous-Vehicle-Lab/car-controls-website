cmake_minimum_required(VERSION 3.10) 
set(CMAKE_CXX_STANDARD 20)
project(server_streaming) 

find_package(PkgConfig REQUIRED)
find_package(Crow CONFIG REQUIRED) 
find_package(OpenCV REQUIRED)

add_executable(STREAMING_SERVER src/main.cpp)

target_include_directories(STREAMING_SERVER PRIVATE headers)
# target_link_libraries(STREAMING_SERVER PRIVATE ${GSTREAMER_LIBRARIES} ${OpenCV_LIBS})
# target_include_directories(STREAMING_SERVER PRIVATE ${CMAKE_SOURCE_DIR}/include)
# target_include_directories(STREAMING_SERVER PRIVATE asio/include) 
target_link_libraries(STREAMING_SERVER PRIVATE Crow::Crow ${GSTREAMER_LIBRARIES} ${OpenCV_LIBS})
