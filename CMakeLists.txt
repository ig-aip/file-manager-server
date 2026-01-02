cmake_minimum_required(VERSION 3.16)

project(file-manager-server LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)

add_executable(file-manager-server
  main.cpp
  server.h server.cpp
  net.h
  client.h client.cpp
)
target_link_libraries(file-manager-server Boost::boost ws2_32 mswsock)

