cmake_minimum_required(VERSION 3.16)

project(file-manager-server LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core)

add_executable(file-manager-server
  main.cpp
  client.cpp client.h


  fileTransfer.cpp fileTransfer.h
  server.cpp server.h
  logger.cpp logger.h
  net.h

)
target_link_libraries(file-manager-server Boost::boost Qt5::Core ws2_32 mswsock)

