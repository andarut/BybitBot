cmake_minimum_required(VERSION 3.23)

project(bybitbot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(CMAKE_C_COMPILER="${CMAKE_C_COMPILER}")
message(CMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}")

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# For Tests
find_package(GTest)
include(GoogleTest)
enable_testing()

add_executable(bybitbot
    ${CMAKE_CURRENT_SOURCE_DIR}/bybit.cpp
)
target_link_libraries(bybitbot PRIVATE
    openssl::openssl
    CURL::libcurl
    # Tests libs
    gtest::gtest
)

# For Tests
# target_compile_definitions(liblena_tests PRIVATE
#     RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/TestsData"
# )
# gtest_discover_tests(liblena_tests)