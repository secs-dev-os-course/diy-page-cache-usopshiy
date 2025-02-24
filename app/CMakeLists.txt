cmake_minimum_required(VERSION 3.12)
project(app
        VERSION 0.0.0
        DESCRIPTION "Application module"
        LANGUAGES C CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning of relative paths translation

add_library(lab2 SHARED IMPORTED)

set_target_properties(lab2 PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/res/liblab2.so)

add_executable(app)
target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR})

target_sources(app
        PRIVATE
        app.cpp # or app.cpp
        tests.cpp
)