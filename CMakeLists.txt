cmake_minimum_required(VERSION 3.10)

# project(check CXX C)

# find boost
find_package(Boost 1.62.0 REQUIRED)
find_package(Threads REQUIRED)
message("boost include directory : ${Boost_INCLUDE_DIRS}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(Boost_USE_MULTITHREADED ON)

set(sources
check.cpp
)

# add_custom_target(check ${CMAKE_CXX_COMPILER} -E check.cpp -I ${Boost_INCLUDE_DIRS} )

add_executable(check ${sources})
target_link_libraries(check ${Boost_LIBRARIES})