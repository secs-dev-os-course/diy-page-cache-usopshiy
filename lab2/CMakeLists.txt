cmake_minimum_required(VERSION 3.12)
project(lab2
        VERSION 0.0.0
        DESCRIPTION "SO module"
        LANGUAGES C CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning of relative paths translation

target_sources(app
        PRIVATE
        lab2.cpp
)

add_library(lab2 SHARED lab2.cpp)

target_include_directories(lab2 PRIVATE ${CMAKE_SOURCE_DIR})

set_target_properties(lab2 PROPERTIES PUBLIC_HEADER lab2.hpp)