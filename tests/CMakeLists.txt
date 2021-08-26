if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()


# include_directories(${PROJECT_HDRS_FOLDER}/include)
# include_directories(${CMAKE_BINARY_DIR})
# link_directories(${Boost_LIBRARY_DIRS})
# link_directories(${CMAKE_BINARY_DIR})
add_executable(
                test-app 
                test_main.cc 
                accounts/test_authorization_presenter.cc
                accounts/test_registration_presenter.cc)

target_link_libraries(test-app PUBLIC CONAN_PKG::catch2 CONAN_PKG::fakeit)


# add_library(Catch2 INTERFACE)
# target_include_directories(Catch2 INTERFACE ${CATCH2_FOLDER_PATH})
# add_library(FakeIt INTERFACE)
# target_include_directories(FakeIt INTERFACE ${FAKEIT_FOLDER_PATH})

# target_link_libraries(test-app
#                        PUBLIC
#                        protocols
#                        spdlog::spdlog
#                        fmt::fmt-header-only
#                        uv_a
#                        libcurl
#                        dl
#                        Catch2
#                        FakeIt
#                        )
# target_compile_features(test-app PRIVATE cxx_std_17)                                       
# target_include_directories(test-app
#                             PUBLIC
#                             ${PROJECT_HDRS_FOLDER}
#                             ${CMAKE_BINARY_DIR}
#                             ${Boost_INCLUDE_DIRS}
                            # )

install(TARGETS test-app DESTINATION bin)
