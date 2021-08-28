include_directories(${CMAKE_BINARY_DIR})
include_directories(${PROJECT_HDRS_FOLDER})
link_directories(${CMAKE_BINARY_DIR})

add_executable(${PROJECT_NAME} 
                main.cc 
                core/networking/asio_socket.cc 
                core/networking/libuv_socket.cc 
                core/messages/protobuf_instruction_handler.cc
                )

target_link_libraries(${PROJECT_NAME}
                       PUBLIC
                       protocols
                       identification
                       responses
                       CONAN_PKG::fmt
                       CONAN_PKG::spdlog
                       CONAN_PKG::cpr
                       CONAN_PKG::asio
                       CONAN_PKG::libuv
                       )
                                       
target_include_directories(${PROJECT_NAME}
                            PUBLIC
                            ${PROJECT_HDRS_FOLDER}
                            ${CMAKE_BINARY_DIR}
                            )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
