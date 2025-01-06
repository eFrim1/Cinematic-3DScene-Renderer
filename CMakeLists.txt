cmake_minimum_required(VERSION 3.23)
cmake_minimum_required(VERSION 3.11)
project(project)
#set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set(CMAKE_CXX_STANDARD 17)
#file(GLOB sources CONFIGURE_DEPENDS *.h *.c)
add_executable(${PROJECT_NAME} main.cpp
        Camera.cpp Camera.hpp Mesh.cpp Mesh.hpp Model3D.cpp Model3D.hpp Shader.cpp Shader.hpp stb_image.cpp stb_image.h tiny_obj_loader.cpp tiny_obj_loader.h SkyBox.hpp SkyBox.cpp)


find_package(OpenGL REQUIRED)

include(FetchContent)
FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)
FetchContent_MakeAvailable(glm)

add_subdirectory (external)

#add_subdirectory(external/glfw-3.4)
include_directories(
        external/glew-2.1.0/include/
        external/glfw-3.4/include/
)

add_definitions(
        -DTW_STATIC
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D
        -DGLEW_STATIC
        -D_CRT_SECURE_NO_WARNINGS
)

set(ALL_LIBS
        ${OPENGL_LIBRARY}
        glfw
        GLEW_210)
target_link_libraries(${PROJECT_NAME} ${ALL_LIBS} glm::glm)

#target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm glfw OpenGL::GL ${GLEW_SOURCE} ${GLEW_HEADERS})

