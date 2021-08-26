# Minimum CMake required
cmake_minimum_required(VERSION 3.14.4)

# Project
project(PROTOBUF_Generations VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# Find required protobuf package
find_package(Protobuf REQUIRED)

# Print version messages
if(Protobuf_FOUND)
 # Find Protobuf installation
   # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
   message(STATUS "Using protobuf ${Protobuf_VERSION}")
   set(Protobuf_USE_STATIC_LIBS ON)
   set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
   set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif ()

# Get the filenames of all the proto files.
file(GLOB PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)

# Generate header and source files for all the proto files
foreach(proto ${PROTO_FILES})
    # Get filename without extension
    get_filename_component(PROTO_NAME_WE ${proto} NAME_WE)
    get_filename_component(PROTO_NAME ${proto} NAME)
    message(STATUS "Generating Protobuf Code for ${PROTO_NAME}")
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto})
    add_library(${PROTO_NAME_WE} STATIC ${PROTO_SRCS} ${PROTO_HDRS})
    target_include_directories(${PROTO_NAME_WE} PUBLIC ${Protobuf_INCLUDE_DIRS})
    if(protobuf_MODULE_COMPATIBLE) #Legacy mode
        target_link_libraries(${PROTO_NAME_WE} ${PROTOBUF_LIBRARIES})
        message(STATUS "GOT LEGACY MODE SUCCESSFULLY")
    else()
        target_link_libraries(${PROTO_NAME_WE} CONAN_PKG::protobuf)
        message(STATUS "GOT NEW MODE SUCCESSFULLY")
    endif()    
endforeach()
