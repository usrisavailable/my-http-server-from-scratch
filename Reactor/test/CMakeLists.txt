cmake_minimum_required(VERSION 3.8)

set(This reactortest)

add_executable(${This} test.cc)
target_link_libraries(${This} PUBLIC
    gtest_main
    Reactor)

add_test(
    NAME ${This}
    COMMAND ${This})        
