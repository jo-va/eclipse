#include "eclipse/render/window.h"
#include "eclipse/util/except.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <cstdlib>

namespace eclipse { namespace render {

Window::Window(unsigned int width, unsigned int height, const char* title)
    : m_window(nullptr)
    , m_window_title(title)
    , m_window_width(width)
    , m_window_height(height)
{
    m_on_key_handler          = [] (int key, int scancode, int action, int mods) { (void)key; (void)scancode; (void)action; (void)mods; };
    m_on_cursor_pos_handler   = [] (double xpos, double ypos)                    { (void)xpos; (void)ypos; };
    m_on_mouse_button_handler = [] (int button, int action, int mods)            { (void)button; (void)action; (void)mods; };
    m_on_scroll_handler       = [] (double xoffset, double yoffset)              { (void)xoffset; (void)yoffset; };
    m_on_resize_handler       = [] (int width, int height)                       { (void)width; (void)height; };
}

Window::~Window()
{
    if (m_window)
        release();
}

void Window::set_key_handler(std::function<void (int key, int scancode, int action, int mods)> h)
{
    if (h)
        m_on_key_handler = h;
}

void Window::set_cursor_pos_handler(std::function<void (double xpos, double ypos)> h)
{
    if (h)
        m_on_cursor_pos_handler = h;
}

void Window::set_mouse_button_handler(std::function<void (int button, int action, int mods)> h)
{
    if (h)
        m_on_mouse_button_handler = h;
}

void Window::set_scroll_handler(std::function<void (double xoffset, double yoffset)> h)
{
    if (h)
        m_on_scroll_handler = h;
}

void Window::set_resize_handler(std::function<void (int width, int height)> h)
{
    if (h)
        m_on_resize_handler = h;
}

void Window::release()
{
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

void Window::swap_buffers()
{
    glfwSwapBuffers(m_window);
}

void Window::show()
{
    glfwShowWindow(m_window);
}

void Window::hide()
{
    glfwHideWindow(m_window);
}

void Window::init()
{
    glfwSetErrorCallback(Window::error_callback);

    if (!glfwInit())
        throw Error("failed to init GLFW");

    //glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, false);
    //glfwWindowHint(GLFW_SAMPLES, 1);

    m_window = glfwCreateWindow(m_window_width, m_window_height, m_window_title, nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, (void*)this);
    const GLFWvidmode* video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(m_window, (video_mode->width - m_window_width) / 2,
                               (video_mode->height - m_window_height) / 2);
    if (!m_window)
        throw Error("failed to create GLFW window");

    glfwMakeContextCurrent(m_window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw Error("failed to initialize GLEW");

    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, true);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetKeyCallback(m_window, Window::key_callback);
    glfwSetCursorPosCallback(m_window, Window::cursor_pos_callback);
    glfwSetMouseButtonCallback(m_window, Window::mouse_button_callback);
    glfwSetScrollCallback(m_window, Window::scroll_callback);
    glfwSetFramebufferSizeCallback(m_window, Window::resize_callback);

    glfwSwapInterval(0);
}

void Window::error_callback(int error, const char* description)
{
    throw Error("GLFW error (" + std::to_string(error) + "): " + std::string(description));
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_on_key_handler(key, scancode, action, mods);
}

void Window::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_on_cursor_pos_handler(xpos, ypos);
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Window* app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_on_mouse_button_handler(button, action, mods);
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

} } // namespace eclipse::render
