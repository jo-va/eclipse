#pragma once

#include "eclipse/render/renderer.h"
#include "eclipse/render/window.h"
#include "eclipse/scene/scene.h"
#include "eclipse/math/vec3.h"

#include <cstdint>
#include <vector>
#include <memory>

namespace eclipse { namespace render {

struct StackedSeries
{
    void init(size_t num_series, size_t max_len);
    void append(size_t index, float value);
    void render(size_t ypos, size_t height);
    void clear();

    std::vector<std::vector<float>> series;
    std::vector<Vec3> colors;
    size_t curr_index;
    size_t total_size;
};

class InteractiveRenderer : public Renderer
{
public:
    InteractiveRenderer(std::shared_ptr<scene::Scene> scene, const Options& options);
    ~InteractiveRenderer();

    int render();

private:
    void init_gl();
    void init_ui();
    void render_ui();
    void on_before_show_ui();

    void key_callback(int key, int scancode, int action, int mods);
    void mouse_button_callback(int button, int action, int mods);
    void cursor_pos_callback(double xpos, double ypos);

private:
    std::unique_ptr<Window> m_window;
    StackedSeries m_series;
    uint32_t m_fbo_id;
    uint32_t m_tex_id;
    bool m_show_ui;
};

} } // namespace eclipse::render
