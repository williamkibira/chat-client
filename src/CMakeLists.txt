include_directories(${CMAKE_BINARY_DIR})
include_directories(${PROJECT_HDRS_FOLDER})
link_directories(${CMAKE_BINARY_DIR})

if(MSVC)
  # target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
  add_compile_options("/std:c++latest")
else()
  target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(${PROJECT_NAME} 
                main.cc 
                core/networking/asio_socket.cc
                core/networking/cpr_http_handler.cc
                core/messages/protobuf_instruction_handler.cc
                )

target_link_libraries(${PROJECT_NAME}
                       PUBLIC
                       protocols
                       identification
                       responses
                       authentication
                       contacts
                       messages
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


