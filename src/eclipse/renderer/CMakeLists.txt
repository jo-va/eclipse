set(RENDERER_HEADERS default_font.h
                     drawing.h
                     gl_util.h
                     window.h)

set(RENDERER_SOURCES window.cpp)

add_library(eclipse_renderer ${RENDERER_SOURCES} ${RENDERER_HEADERS})
target_link_libraries(eclipse_renderer ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} glfw ${GLFW_LIBRARIES})
