cmake_minimum_required(VERSION 3.8)

set(This testdemo)

add_executable(${This} testdemo.cc)
target_link_libraries(${This} PUBLIC
    gtest_main
    Protocol
    Reactor)

add_test(
    NAME ${This}
    COMMAND ${This})        
