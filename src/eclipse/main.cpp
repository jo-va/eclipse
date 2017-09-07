#include "eclipse/renderer/window.h"
#include "eclipse/renderer/drawing.h"
#include "eclipse/util/stop_watch.h"
#include "eclipse/util/logger.h"
#include "eclipse/util/resource.h"
#include "eclipse/util/file_util.h"
#include "eclipse/util/input_parser.h"
#include "eclipse/scene/scene.h"
#include "eclipse/scene/scene_io.h"

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdint>

using namespace eclipse;

namespace {
    auto logger = Logger::create("main");
}

int main(int argc, char** argv)
{
    InputParser input(argc, argv);

    if (input.option_exists("--help") || argc == 1)
    {
        std::cout << "Eclipse - Path tracing renderer\n\n"
                  << "usage: eclipse --help\n"
                  << "usage: eclipse --info [scene.(obj|bin)]\n"
                  << "usage: eclipse --compile [scene.obj]\n"
                  << "usage: eclipse --list-devices\n"
                  << "usage: eclipse --render [scene.(obj|bin)]\n\n"
                  << "options in order of precedence:\n"
                  << "       --help         Print this menu\n"
                  << "       --info         Print scene statistics\n"
                  << "       --list-devices List the available rendering devices\n"
                  << "       --compile      Compile scene to a compressed binary format\n"
                  << "       --render       Render a scene\n";
    }
    else if (input.option_exists("--info"))
    {
        try
        {
            std::string scene_file = input.get_option("--info");
            if (!scene_file.empty())
            {
                std::shared_ptr<Resource> scene_res = std::make_shared<Resource>(scene_file);
                std::shared_ptr<scene::Scene> scene = scene::read(scene_res);

                std::string stats = scene->get_stats();
                logger.log<INFO>(stats);
            }
            else
            {
                logger.log<WARNING>("no scene specified");
            }
        }
        catch (std::exception& e)
        {
            logger.log<ERROR>("Exception caught: ", e.what());
        }
    }
    else if (input.option_exists("--list-devices"))
    {
    }
    else if (input.option_exists("--compile"))
    {
        try
        {
            std::string scene_file = input.get_option("--compile");
            if (!scene_file.empty())
            {
                std::shared_ptr<Resource> scene_res = std::make_shared<Resource>(scene_file);
                if (has_extension(scene_res->get_path(), ".bin"))
                {
                    logger.log<WARNING>("scene ", scene_res->get_path(), " already compiled");
                }
                else
                {
                    std::shared_ptr<scene::Scene> scene = scene::read(scene_res);
                    scene::write(scene, scene_res);
                }
            }
            else
            {
                logger.log<WARNING>("no scene specifed");
            }
        }
        catch (std::exception& e)
        {
            logger.log<ERROR>("Exception caught: ", e.what());
        }
    }
    else if (input.option_exists("--render"))
    {
    }

    /*
    Window window(g_width, g_height, "Eclipse renderer");

    bool pause = false;
    window.set_key_handler([&](int key, int scancode, int action, int mods) {
        (void)scancode;
        (void)mods;
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
            pause ^= 1;
    });

    if (!window.init())
    {
        std::cerr << "can't initialize window" << std::endl;
        exit(EXIT_FAILURE);
    }

    uint32_t* accum = new uint32_t[g_width * g_height];
    draw::Buffer<uint32_t> buf = { accum, g_width, g_height, g_width };
    draw::clear(buf, draw::Point<uint32_t>(0, 0), draw::Point<uint32_t>(g_width, g_height), 0u);

    int num_frames = 0;
    int last_num_frames = 0;
    std::stringstream ss;
    StopWatch timer;
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
    */

    return 0;
}
