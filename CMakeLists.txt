cmake_minimum_required(VERSION 3.16)

project(file-manager-server LANGUAGES CXX)
#-DCMAKE_TOOLCHAIN_FILE=/home/igore/Documents/vcpkg/scripts/buildsystems/vcpkg.cmake

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)



#add_subdirectory(tests)
add_library(server
  client.cpp client.h
  fileTransfer.cpp fileTransfer.h
  server.cpp server.h
  logger.cpp logger.h
  net.h
  tcpHeader.h)

target_link_libraries(server PUBLIC Boost::boost Qt6::Core)


add_executable(server_tests
  server_test_suit.cpp)
target_link_libraries(server_tests PRIVATE
  GTest::gtest
  GTest::gtest_main
  server
  )

add_executable(file-manager-server
  main.cpp
)
target_link_libraries(file-manager-server PRIVATE server)
