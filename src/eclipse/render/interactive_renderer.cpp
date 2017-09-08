#include "eclipse/render/interactive_renderer.h"
#include "eclipse/scene/scene.h"
#include "eclipse/scene/camera.h"
#include "eclipse/render/options.h"
#include "eclipse/render/window.h"
#include "eclipse/math/vec3.h"
#include "eclipse/math/math.h"

#include <memory>
#include <string>
#include <sstream>
#include <GL/glew.h>

namespace eclipse { namespace render {

constexpr uint32_t StackedSeriesHeight = 20;

void check_gl_error()
{
    GLenum err;
    std::ostringstream oss;
    while ((err = glGetError()) != GL_NO_ERROR)
        oss << "(" << err << "): " << glewGetErrorString(err);
    if (!oss.str().empty())
        throw Error("OpenGL error: " + oss.str());
}

InteractiveRenderer::InteractiveRenderer(std::shared_ptr<scene::Scene> scene, const Options& options)
    : Renderer(scene, options), m_show_ui(false)
{
    m_window = std::make_unique<Window>(m_options.frame_width, m_options.frame_height, "Eclipse renderer");

    scene->camera.make_projection((float)m_options.frame_width / (float)m_options.frame_height);
    scene->camera.invert_y_axis(true);

    m_window->init();

    m_window->set_key_handler([&](int key, int scancode, int action, int mods) {
        key_callback(key, scancode, action, mods);
    });
    m_window->set_cursor_pos_handler([&](double xpos, double ypos) {
        cursor_pos_callback(xpos, ypos);
    });
    m_window->set_mouse_button_handler([&] (int button, int action, int mods) {
        mouse_button_callback(button, action, mods);
    });

    init_gl();
    init_ui();
    m_window->show();
}

InteractiveRenderer::~InteractiveRenderer()
{
    if (m_fbo_id)
        glDeleteFramebuffers(1, &m_fbo_id);
    if (m_tex_id)
        glDeleteTextures(1, &m_tex_id);
    m_fbo_id = m_tex_id = 0;
}

int InteractiveRenderer::render()
{
    while (!m_window->should_close())
    {
        m_window->poll_events();

        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_id);
        glBlitFramebuffer(0, 0, m_options.frame_width, m_options.frame_height,
                          0, 0, m_options.frame_width, m_options.frame_height,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (m_show_ui)
            render_ui();

        m_window->swap_buffers();
    }
    return 0;
}

void InteractiveRenderer::init_gl()
{
    glGenTextures(1, &m_tex_id);
    glBindTexture(GL_TEXTURE_2D, m_tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_options.frame_width, m_options.frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &m_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_id, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw Error("OpenGL framebuffer setup failed");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    check_gl_error();
}

void InteractiveRenderer::init_ui()
{
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_options.frame_width, m_options.frame_height, 0, -1, 1);
    glViewport(0, 0, m_options.frame_width, m_options.frame_height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    check_gl_error();

    m_series.init(m_tracers.size(), m_options.frame_width);
}

void InteractiveRenderer::on_before_show_ui()
{
    m_series.clear();
}

void InteractiveRenderer::render_ui()
{
    for (size_t index = 0; index < m_blocks.size(); ++index)
        m_series.append(index, float(m_blocks[index]));

    m_series.render(m_options.frame_height - StackedSeriesHeight, StackedSeriesHeight);
}

void InteractiveRenderer::key_callback(int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        m_window->set_should_close(true);
        break;
    case GLFW_KEY_TAB:
        m_show_ui = !m_show_ui;
        if (m_show_ui)
            on_before_show_ui();
        break;
    default:
        return;
    }

    (void)scancode;
    (void)mods;
}

void InteractiveRenderer::mouse_button_callback(int button, int action, int mods)
{
    (void)button;
    (void)action;
    (void)mods;
}

void InteractiveRenderer::cursor_pos_callback(double xpos, double ypos)
{
    (void)xpos;
    (void)ypos;
}

void StackedSeries::init(size_t num_series, size_t max_len)
{
    curr_index = 0;
    total_size = max_len;

    series.resize(num_series);
    colors.resize(num_series);

    for (size_t i = 0; i < num_series; ++i)
        colors[i] = Vec3(random<float>(), random<float>(), 1.0);
}

void StackedSeries::append(size_t index, float value)
{
    if (series[index].size() < total_size)
        series[index].push_back(value);
    else
        series[index][curr_index % total_size] = value;
    ++curr_index;
}

void StackedSeries::clear()
{
    curr_index = 0;
    for (size_t i = 0; i < series.size(); ++i)
        series[i].clear();
}

void StackedSeries::render(size_t ypos, size_t height)
{
    if (series.empty())
        return;

    glBegin(GL_LINES);
    for (size_t x = 0; x < series[0].size(); ++x)
    {
        float sum = 0.0f;
        float scale = 1.0f;
        for (size_t index = 0; index < series.size(); ++index)
            sum += series[index][x];
        if (sum > 0.0f)
            scale = float(height) / sum;

        float y = float(ypos);
        glLineWidth(1.0);
        for (size_t index = 0; index < series.size(); ++index)
        {
            float h = series[index][x] * scale;
            glColor3fv(&colors[index][0]);
            glVertex2f(float(x), y);
            glVertex2f(float(x), y + h);
            y += h;
        }
    }
    glEnd();
}

} } // namespace eclipse::render
