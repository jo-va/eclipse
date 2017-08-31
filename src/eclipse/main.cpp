#include "eclipse/renderer/window.h"
#include "eclipse/renderer/drawing.h"
#include "eclipse/util/timer.h"
#include "eclipse/util/logger.h"
#include "eclipse/scene/resource.h"
#include "eclipse/scene/obj_reader.h"

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdint>

using namespace eclipse;

namespace {
    const uint32_t g_width = 640;
    const uint32_t g_height = 512;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    if (argc < 2)
        return 0;

    Resource scene_res(argv[1]);//"https://raw.githubusercontent.com/achilleasa/polaris-example-scenes/master/diamonds/diamonds.obj");
    OBJReader reader;
    Scene* scene = reader.read(&scene_res);
    delete scene;

    Window window(g_width, g_height, "Pathtracer");

    bool pause = false;
    window.set_key_handler([&](int key, int scancode, int action, int mods) {
        (void)scancode;
        (void)mods;
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
            pause ^= 1;
    });

    if (!window.init())
    {
        std::cerr << "Can't initialize window" << std::endl;
        exit(EXIT_FAILURE);
    }

    uint32_t* accum = new uint32_t[g_width * g_height];
    draw::Buffer<uint32_t> buf = { accum, g_width, g_height, g_width };
    draw::clear(buf, draw::Point<uint32_t>(0, 0), draw::Point<uint32_t>(g_width, g_height), 0u);

    int num_frames = 0;
    int last_num_frames = 0;
    std::stringstream ss;
    Timer timer;
    timer.start();

    while (!window.should_close())
    {
        window.poll_events();
        if (pause)
            continue;

        for (uint32_t i = 0; i < g_height; ++i)
            for (uint32_t j = 0; j < g_width; ++j)
                accum[i * g_width + j] = rand();

        buf = { (uint32_t*)window.get_framebuffer(), g_width, g_height, g_width };

        // Copy image to framebuffer
        memcpy(buf.buffer, accum, g_width * g_height * sizeof(uint32_t));

        if (++num_frames % 20 == 0)
        {
            timer.stop();
            double dt = timer.get_elapsed_time_us();
            timer.start();

            ss.str("");
            ss << int(double(num_frames - last_num_frames) * 1000000.0 / dt) << " fps";
            last_num_frames = num_frames;
        }

        draw::bar(buf, draw::Point<uint32_t>(0, g_height - 25), draw::Point<uint32_t>(200, g_height), 0u);
        draw::print(buf, ss.str().c_str(), { 10, g_height - 20 }, 0xffffffff);

        // Show new framebuffer
        window.flush_framebuffer();
    }

    delete[] accum;
    window.release();

    return 0;
}
