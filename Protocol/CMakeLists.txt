cmake_minimum_required(VERSION 3.8)

set(This Protocol)

set(Headers 
    include/Protocol/Protocol.hpp)

set(Sources 
    src/Protocol.cc)

add_library(${This} STATIC
    ${Headers} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)
target_link_libraries(${This} PUBLIC Reactor)  
target_include_directories(${This} PUBLIC include)

#add_subdirectory(test)
