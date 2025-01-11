
if(MSVC AND NOT "${MSVC_VERSION}" LESS 1400)
    add_definitions( "/MP" )
endif()


add_definitions(
        -DTW_STATIC
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D
        -DGLEW_STATIC
        -D_CRT_SECURE_NO_WARNINGS
)

add_subdirectory (glfw-3.4)
add_subdirectory (glm)

include_directories(
        glfw-3.4/include/GLFW/
        glew-2.1.0/include/
        glm/
)

set(GLEW_SOURCE glew-2.1.0/src/glew.c)
set(GLEW_HEADERS glew-2.1.0/include/GL)

add_library( GLEW_210 STATIC
        ${GLEW_SOURCE}
        ${GLEW_HEADERS}
        )

target_link_libraries(GLEW_210
        ${OPENGL_LIBRARY}
        ${EXTRA_LIBS}
        )