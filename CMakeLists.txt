# This file is based on https://github.com/krux02/minimal_cmake_example/blob/master/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(vulkan_rogue)

###############################################################################
## file globbing ##############################################################
###############################################################################

file(GLOB_RECURSE sources src/*.c src/*.h extern/vma.cpp)

###############################################################################
## target definitions #########################################################
###############################################################################

add_executable(${PROJECT_NAME} ${sources})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "-std=c99 -Wall -pedantic -Wfloat-conversion -Wno-misleading-indentation")
target_include_directories(${PROJECT_NAME} PUBLIC src/ extern/)

###############################################################################
## dependencies ###############################################################
###############################################################################

find_package(Vulkan REQUIRED)

# This is a workaround for msys/mingw not playing nice with glfw
if (MINGW)
    target_link_libraries(${PROJECT_NAME} PUBLIC
            glfw3
            gdi32
            ${Vulkan_LIBRARIES})
else()
    find_package(glfw3 3.3 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC
            glfw
            ${Vulkan_LIBRARIES})
endif()
