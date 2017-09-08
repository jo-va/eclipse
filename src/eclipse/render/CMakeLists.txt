set(RENDER_HEADERS options.h
                   interactive_renderer.h
                   default_font.h
                   drawing.h
                   gl_util.h
                   window.h)

set(RENDER_SOURCES interactive_renderer.cpp
                   window.cpp)

add_library(eclipse_render ${RENDER_SOURCES} ${RENDER_HEADERS})
target_link_libraries(eclipse_render ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} glfw ${GLFW_LIBRARIES})
