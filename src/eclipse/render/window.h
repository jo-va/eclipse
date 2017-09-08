#pragma once

#include <functional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window
{
public:
    Window(unsigned int width, unsigned int height, const char* title);
    virtual ~Window();

    bool init();
    bool should_close();
    void poll_events();
    void* get_framebuffer();
    void flush_framebuffer();
    void release();
    void set_should_close(bool value = true);

    void set_key_handler(std::function<void (int key, int scancode, int action, int mods)> h) { if (h) m_on_key_handler = h; }
    void set_mouse_handler(std::function<void (double xpos, double ypos)> h) { if (h) m_on_mouse_handler = h; }
    void set_scroll_handler(std::function<void (double xoffset, double yoffset)> h) { if (h) m_on_scroll_handler = h; }
    void set_resize_handler(std::function<void (int width, int height)> h) { if (h) m_on_resize_handler = h; }

private:
    static void error_callback(int error, const char* description);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void resize_callback(GLFWwindow* window, int width, int height);

    bool init_window();
    bool init_gl();
    void create_textured_quad();

protected:
    GLFWwindow* m_window;
    unsigned int m_window_width;
    unsigned int m_window_height;
    unsigned int m_render_width;
    unsigned int m_render_height;
    unsigned int m_texture_id;
    unsigned int m_pbo_ids[2];
    void* m_fb_data;
    const char* m_window_title;
    unsigned int m_quad_vao_id;
    unsigned int m_quad_vbo_id;
    unsigned int m_quad_program_id;

    std::function<void (int, int, int, int)> m_on_key_handler;
    std::function<void (double, double)> m_on_mouse_handler;
    std::function<void (double, double)> m_on_scroll_handler;
    std::function<void (int, int)> m_on_resize_handler;
};
