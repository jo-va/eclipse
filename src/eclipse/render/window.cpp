#include "eclipse/render/window.h"
#include "eclipse/render/gl_util.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstdlib>

Window::Window(unsigned int width, unsigned int height, const char* title)
    : m_window(nullptr)
    , m_window_width(width)
    , m_window_height(height)
    , m_render_width(width)
    , m_render_height(height)
    , m_texture_id(0)
    , m_fb_data(nullptr)
    , m_window_title(title)
    , m_quad_vao_id(0)
    , m_quad_vbo_id(0)
    , m_quad_program_id(0)
{
    m_pbo_ids[0] = m_pbo_ids[1] = 0;

    m_on_key_handler    = [] (int key, int scancode, int action, int mods) { (void)key; (void)scancode; (void)action; (void)mods; };
    m_on_mouse_handler  = [] (double xpos, double ypos)                    { (void)xpos; (void)ypos; };
    m_on_scroll_handler = [] (double xoffset, double yoffset)              { (void)xoffset; (void)yoffset; };
    m_on_resize_handler = [] (int width, int height)                       { (void)width; (void)height; };
}

Window::~Window()
{
    if (m_window)
        release();
}

bool Window::init()
{
    if (!init_window()) {
        release();
        std::cerr << "Failed to init window" << std::endl;
        return false;
    }

    if (!init_gl()) {
        release();
        std::cerr << "Can't initialize OpenGL" << std::endl;
        return false;
    }

    return true;
}

void Window::release()
{
    if (m_quad_vao_id) glDeleteVertexArrays(1, &m_quad_vao_id);
    if (m_quad_vbo_id) glDeleteBuffers(1, &m_quad_vbo_id);
    if (m_quad_program_id) glDeleteProgram(m_quad_program_id);
    if (m_pbo_ids[0] || m_pbo_ids[1]) glDeleteBuffers(2, m_pbo_ids);
    if (m_texture_id) glDeleteTextures(1, &m_texture_id);

    m_quad_vao_id = 0;
    m_quad_vbo_id = 0;
    m_quad_program_id = 0;
    m_pbo_ids[0] = m_pbo_ids[1] = 0;
    m_texture_id = 0;

    if (m_window)
        glfwDestroyWindow(m_window);
    m_window = nullptr;
    glfwTerminate();
}

bool Window::should_close()
{
    return glfwWindowShouldClose(m_window);
}

void Window::set_should_close(bool value)
{
    glfwSetWindowShouldClose(m_window, (int)value);
}

void Window::poll_events()
{
    glfwPollEvents();
}

void* Window::get_framebuffer()
{
    static int index = 0;
    index = (index + 1) % 2;
    int next_index = (index + 1) % 2;

    // Bind the texture and PBO
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo_ids[index]);

    // Copy pixels from PBO to texture object on GPU, use offset instead of pointer
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_window_width, m_window_height, GL_BGRA, GL_UNSIGNED_BYTE, 0);

    // Bind PBO to update pixel values
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo_ids[next_index]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * m_window_width * m_window_height, 0, GL_STREAM_DRAW);

    m_fb_data = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

    return m_fb_data;
}

void Window::flush_framebuffer()
{
    if (m_fb_data) {
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // Draw texture to fullscreen quad
    glUseProgram(m_quad_program_id);
    glBindVertexArray(m_quad_vao_id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glfwSwapBuffers(m_window);
}

bool Window::init_gl()
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Glew failed to init" << std::endl;
        return false;
    }

    // Streaming texture upload using PBOs enabling DMA texture transfers to the GPU
    // http://www.songho.ca/opengl/gl_pbo.html

    // Create the framebuffer texture and reserve space for it
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_window_width, m_window_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERROR();

    // Create two pixel buffer objects and reserve memory space for them
    glGenBuffers(2, m_pbo_ids);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo_ids[0]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * m_window_width * m_window_height, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo_ids[1]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * m_window_width * m_window_height, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    CHECK_GL_ERROR();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glViewport(0, 0, m_window_width, m_window_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL_ERROR();

    create_textured_quad();

    return true;
}

void Window::create_textured_quad()
{
    const char* quad_vs_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec2 position;\n"
        "layout (location = 1) in vec2 texcoord;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 0.0, 1.0);\n"
        "    TexCoord = texcoord;\n"
        "}";

    const char* quad_fs_shader_source =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D tex;\n"
        "void main() {\n"
        "    FragColor = texture(tex, TexCoord);\n"
        "}";

    const float vertex_data[] = {
        -1.0f,-1.0f, 0.0f, 0.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f,-1.0f, 0.0f, 0.0f,
         1.0f,-1.0f, 1.0f, 0.0f,
         1.0f, 1.0f, 1.0f, 1.0f
    };
    glGenVertexArrays(1, &m_quad_vao_id);
    glGenBuffers(1, &m_quad_vbo_id);

    glBindVertexArray(m_quad_vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    m_quad_program_id = glCreateProgram();
    GLuint vshader = create_shader(GL_VERTEX_SHADER, quad_vs_shader_source);
    GLuint fshader = create_shader(GL_FRAGMENT_SHADER, quad_fs_shader_source);
    glAttachShader(m_quad_program_id, vshader);
    glAttachShader(m_quad_program_id, fshader);
    glLinkProgram(m_quad_program_id);

    GLint linked_ok = GL_FALSE;
    glGetProgramiv(m_quad_program_id, GL_LINK_STATUS, &linked_ok);
    if (linked_ok == GL_FALSE) {
        std::cerr << "Quad program error: " << std::endl;
        print_log(m_quad_program_id);
        return;
    }

    glDeleteShader(vshader);
    glDeleteShader(fshader);

    glUseProgram(m_quad_program_id);
    glUniform1i(glGetUniformLocation(m_quad_program_id, "tex"), 0);
    glUseProgram(0);

    CHECK_GL_ERROR();
}

bool Window::init_window()
{
    glfwSetErrorCallback(Window::error_callback);

    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW" << std::endl;
        return false;
    }

    //glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);
    //glfwWindowHint(GLFW_SAMPLES, 1);

    m_window = glfwCreateWindow(m_window_width, m_window_height, m_window_title, nullptr, nullptr);

    const GLFWvidmode* video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    glfwSetWindowPos(m_window, (video_mode->width - m_window_width) / 2,
                               (video_mode->height - m_window_height) / 2);

    if (!m_window) {
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }

    glfwSetWindowUserPointer(m_window, (void*)this);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0);

    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, true);
    //glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetKeyCallback(m_window, Window::key_callback);
    glfwSetCursorPosCallback(m_window, Window::mouse_callback);
    glfwSetScrollCallback(m_window, Window::scroll_callback);
    glfwSetFramebufferSizeCallback(m_window, Window::resize_callback);

    glfwShowWindow(m_window);

    return true;
}

void Window::error_callback(int error, const char* description)
{
    (void)error;
    std::cerr << "GLFW error: " << description << std::endl;
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);

    Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_on_key_handler(key, scancode, action, mods);
}

void Window::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_on_mouse_handler(xpos, ypos);
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_on_scroll_handler(xoffset, yoffset);
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
    Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_on_resize_handler(width, height);
}
