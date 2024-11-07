# This file is based on https://github.com/krux02/minimal_cmake_example/blob/master/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(vulkan_rogue)

if(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
   include_directories(SYSTEM "/usr/include/c++/v1")
   include_directories("/usr/X11R6/include")
   link_directories("/usr/X11R6/lib")
endif()

###############################################################################
## file globbing ##############################################################
###############################################################################

file(GLOB_RECURSE sources src/*.c src/*.h extern/vma.cpp)

###############################################################################
## Functions ##################################################################
###############################################################################
# Shader compilation function
function(compile_shader SHADER_TYPE)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/shaders/${SHADER_TYPE}.spv
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_SOURCE_DIR}/shaders/shader.${SHADER_TYPE} -o ${CMAKE_BINARY_DIR}/shaders/${SHADER_TYPE}.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/shaders/shader.${SHADER_TYPE}
        COMMENT "Compiling ${SHADER_TYPE}"
    )
endfunction()

###############################################################################
## target definitions #########################################################
###############################################################################

add_executable(${PROJECT_NAME} ${sources})
add_dependencies(${PROJECT_NAME} shaders)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-g -O0 -std=c99 -Wall -pedantic -Wfloat-conversion -Wno-misleading-indentation -Wno-nullability-extension")
target_include_directories(${PROJECT_NAME} PUBLIC src/ extern/)

compile_shader(vert)
compile_shader(frag)

# Add compiled shaders to a custom target
add_custom_target(shaders
    DEPENDS
        ${CMAKE_BINARY_DIR}/shaders/vert.spv
        ${CMAKE_BINARY_DIR}/shaders/frag.spv
)

###############################################################################
## dependencies ###############################################################
###############################################################################

find_package(Vulkan REQUIRED)
find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)

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
