# version required for HIP language support
# otherwise, version 3.18 is enough (but I don't recall for what function)
cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project("Kokkidio" VERSION 1.0.0)

message(STATUS "Using CXX compiler: ${CMAKE_CXX_COMPILER}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

set(PROJ_CONF_DIR "${CMAKE_SOURCE_DIR}/cmake")
include(${PROJ_CONF_DIR}/include_conf.cmake)
enable_language(CXX)
include_conf(checkVar)
include_conf(checkKokkos)
include_conf(checkEigen)
include_conf(sharedOpts)
include_conf(configureTarget)
include_conf(install)




add_subdirectory(src/examples)
enable_testing()
add_subdirectory(src/tests)