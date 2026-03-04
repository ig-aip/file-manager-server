cmake_minimum_required(VERSION 3.16)

project(file-manager-server LANGUAGES CXX)


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)



#add_subdirectory(tests)
add_library(server


  server.cpp server.h

  net.h
  tcpHeader.h
  session.h session.cpp
  consolelogger.h
  consolelogger.cpp)

target_link_libraries(server PUBLIC Boost::boost)




if(WIN32)
  target_link_libraries(server PUBLIC ws2_32 mswsock)


elseif(UNIX)
  target_link_libraries(server PUBLIC pthread)
endif()


if(MSVC)
  add_executable(server_tests
    server_test_suit.cpp)
  target_link_libraries(server_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    server
    )
endif()


add_executable(file-manager-server
  main.cpp
)
target_link_libraries(file-manager-server PRIVATE server)

target_sources(file-manager-server
  PRIVATE

)









