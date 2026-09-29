cmake_minimum_required(VERSION 3.30)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
project(FluidGL)

option(BUILD_TEST "Run through the tests" OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")
endif()

# Compile glad
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Compile imgui
file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/imgui/*.cpp)
list(FILTER IMGUI_SOURCES EXCLUDE REGEX ".*freetype.*")
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/include/imgui)
target_link_libraries(imgui PUBLIC glfw)

# Load OpenMP
find_package(OpenMP REQUIRED)

function(set_common_target_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /EHsc /openmp)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -fopenmp)
    endif()
    target_compile_features(${target} PRIVATE cxx_std_17)
endfunction()

# Get libraries directly from their repo
include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(nfd GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git GIT_TAG v1.3.0)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.5.1)
FetchContent_MakeAvailable(nfd glfw)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM CORE_SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

add_library(fluidsim_core STATIC ${CORE_SOURCES})
set_common_target_options(fluidsim_core)
target_include_directories(fluidsim_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(fluidsim_core PUBLIC glad imgui glfw nfd OpenMP::OpenMP_CXX)

if (BUILD_TEST)
    FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.16.0)
    FetchContent_MakeAvailable(Catch2)

    file(GLOB_RECURSE TESTS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*.cpp")
    add_executable(buildtests ${TESTS})
    set_common_target_options(buildtests)
    target_include_directories(buildtests PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(buildtests PRIVATE Catch2::Catch2 fluidsim_core)

    enable_testing()
    include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
    catch_discover_tests(buildtests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}) 
endif()

add_executable(myprogram src/main.cpp)
set_common_target_options(myprogram)
target_link_libraries(myprogram PRIVATE fluidsim_core)