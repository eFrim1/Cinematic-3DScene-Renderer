cmake_minimum_required(VERSION 3.11)
project(project)

set(CMAKE_CXX_STANDARD 17)
file(GLOB sources CONFIGURE_DEPENDS *.hpp *.cpp)
add_executable(${PROJECT_NAME} ${sources})

find_package(OpenGL REQUIRED)

add_subdirectory (external)

include_directories(
        external/glew-2.1.0/include/
        external/glfw-3.4/include/
        external/glm
)

add_definitions(
        -DTW_STATIC
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D
        -DGLEW_STATIC
        -D_CRT_SECURE_NO_WARNINGS
        -DGLM_ENABLE_EXPERIMENTAL
)

set(ALL_LIBS
        ${OPENGL_LIBRARY}
        glfw
        GLEW_210
        glm)

target_link_libraries(${PROJECT_NAME} ${ALL_LIBS})


