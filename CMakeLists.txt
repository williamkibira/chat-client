cmake_minimum_required(VERSION 3.15)

project(cpp-chat-client LANGUAGES CXX)

add_compile_options(-std=c++17)

include(${CMAKE_CURRENT_LIST_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PROJECT_VERSION "1.0.0")
set(CMAKE_PROJECT_VERSION_MAJOR 1)
set(CMAKE_PROJECT_VERSION_MINOR 0)
set(CMAKE_PROJECT_VERSION_PATCH 0)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(BUILD_TESTS "Build Unit tests" ON)
set(BUILD_TESTS ON)

find_package(SQLite3 REQUIRED)

if(SQLite3_FOUND)
  message(STATUS "Found Sqlite ${SQLite3_VERSION}")
else()
  message(WARNING "Could not find Sqlite 3")
endif()



#########################################################
# ADDING COMPILED PROTOCOL BUFFERS COMPILATION          #
#########################################################

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/protos")


##################################################################################
# References to source folders in project                                        #
##################################################################################

set(PROJECT_SRC_FOLDER "${CMAKE_CURRENT_LIST_DIR}/src")
set(PROJECT_HDRS_FOLDER "${CMAKE_CURRENT_LIST_DIR}/include")

##################################################################################
# Adding the actual project source folder                                        #
##################################################################################
add_subdirectory("${PROJECT_SRC_FOLDER}")
include_directories("${PROJECT_HDRS_FOLDER}")


##################################################################################
# Project Unit Tests                                                             #
##################################################################################

if(BUILD_TESTS)
#  set(CATCH2_FOLDER_PATH "${CMAKE_CURRENT_LIST_DIR}/third_party/catch2")
#  set(FAKEIT_FOLDER_PATH "${CMAKE_CURRENT_LIST_DIR}/third_party/fakeit")
 enable_testing() 
 add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/tests")
endif()
