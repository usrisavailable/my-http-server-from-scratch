cmake_minimum_required(VERSION 3.8)

set(This protocoltest)

add_executable(${This} test.cc)
target_link_libraries(${This} PUBLIC
    gtest_main
    Protocol)

add_test(
    NAME ${This}
    COMMAND ${This})        
