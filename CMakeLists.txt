cmake_minimum_required(VERSION 3.15)
project(ComputerGraphics)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Define source files
set(SOURCES
    main.cpp
    libraries/lib/glad.c
    src/util.cpp
    src/sphere.cpp
    src/skybox.cpp
)

# Define shader files (optional grouping for IDE visibility)
file(GLOB SHADER_FILES "shader/*.glsl")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${SHADER_FILES})

# Include directories
include_directories(
    libraries/include
    include
)

# Link GLFW and OpenGL
find_package(OpenGL REQUIRED)

# Link GLFW3 manually (adjust path as needed)
target_link_directories(${PROJECT_NAME} PRIVATE libraries/lib)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} glfw3 OpenGL::GL)
else()
    target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL)
endif()

# Copy all shaders and resources over
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shader"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shader"
    COMMENT "Copying shaders to the build directory"
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    COMMENT "Copying resources to the build directory"
)

# Preprocessor definitions for static linking of GLFW
target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

# Platform-specific settings for Windows
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:CONSOLE)
endif()
