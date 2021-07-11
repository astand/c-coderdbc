cmake_minimum_required(VERSION 3.5)

project(coderdbc-cli LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(coderdbc
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/c-main-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/c-util-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/conditional-tree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/c-sigprinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/filewriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/fs-creator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers/formatter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/dbclineparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/dbcscanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/maincli.cpp
)
