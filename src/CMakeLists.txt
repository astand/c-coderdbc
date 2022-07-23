cmake_minimum_required(VERSION 3.5)

project(coderdbc-cli LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(coderdbc
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/c-main-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/mon-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/config-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/c-util-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/conditional-tree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/c-sigprinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/filewriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/fs-creator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers/formatter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/dbclineparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/dbcscanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/options-parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/maincli.cpp
)


include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  cctest
  ${CMAKE_CURRENT_SOURCE_DIR}/helpers/formatter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/options-parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/args-test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/bitext-test.cpp
)

target_link_libraries(
  cctest
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cctest)
