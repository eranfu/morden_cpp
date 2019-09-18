cmake_minimum_required(VERSION 3.14)
project(morden_cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(main main.cpp)
add_executable(e2.1 exercises/c2/2.1.cpp)
add_executable(e2.2 exercises/c2/2.2.cpp)

add_executable(e6_http exercises/c6/main_http.cpp)
target_include_directories(e6_http PRIVATE exercises/c6/)
target_link_libraries(e6_http ws2_32 boost_system-mt wsock32)

add_executable(e6_https exercises/c6/main_https.cpp)
target_include_directories(e6_https PRIVATE exercises/c6/)
target_link_libraries(e6_https ws2_32 boost_system-mt wsock32)