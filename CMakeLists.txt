cmake_minimum_required(VERSION 3.21)
project(LinSpire VERSION 0.2.0 LANGUAGES CXX)

include(CTest)
enable_testing()

add_library(LinSpire src/linspire.cpp src/var.cpp)
target_compile_features(LinSpire PUBLIC cxx_std_17)
target_include_directories(LinSpire PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
if(NOT TARGET json)
    add_subdirectory(extern/json)
endif()
add_dependencies(LinSpire json)
target_link_libraries(LinSpire PUBLIC json)
setup_sanitizers(LinSpire)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(CPACK_PROJECT_NAME LinSpire)
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
