#pragma once

#include <cstdint>
#include <functional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace eclipse { namespace render {

class Window
{
public:
    Window(unsigned int width, unsigned int height, const char* title);
    virtual ~Window();

    void init();
    void release();

    bool should_close();
    void set_should_close(bool value = true);

    void poll_events();
    void swap_buffers();
    void show();
    void hide();

    void set_key_handler(std::function<void (int key, int scancode, int action, int mods)> h);
    void set_cursor_pos_handler(std::function<void (double xpos, double ypos)> h);
    void set_mouse_button_handler(std::function<void (int button, int action, int mods)> h);
    void set_scroll_handler(std::function<void (double xoffset, double yoffset)> h);
    void set_resize_handler(std::function<void (int width, int height)> h);

private:
    static void error_callback(int error, const char* description);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void resize_callback(GLFWwindow* window, int width, int height);

protected:
    GLFWwindow* m_window;
    const char* m_window_title;
    unsigned int m_window_width;
    unsigned int m_window_height;

    std::function<void (int, int, int, int)> m_on_key_handler;
    std::function<void (double, double)> m_on_cursor_pos_handler;
    std::function<void (int, int, int)> m_on_mouse_button_handler;
    std::function<void (double, double)> m_on_scroll_handler;
    std::function<void (int, int)> m_on_resize_handler;
};

} } // namespace eclipse::render
