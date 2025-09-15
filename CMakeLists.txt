cmake_minimum_required(VERSION 3.21)
project(linspire VERSION 0.2.0 LANGUAGES CXX)

include(CTest)
enable_testing()

add_library(linspire src/linspire.cpp)
target_compile_features(linspire PUBLIC cxx_std_17)
target_include_directories(linspire PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
if(NOT TARGET utils)
    add_subdirectory(extern/utils)
endif()
add_dependencies(linspire utils)
target_link_libraries(linspire PUBLIC utils)
setup_sanitizers(linspire)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(CPACK_PROJECT_NAME linspire)
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
