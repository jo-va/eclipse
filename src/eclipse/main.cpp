#include "eclipse/util/logger.h"
#include "eclipse/util/resource.h"
#include "eclipse/util/file_util.h"
#include "eclipse/util/input_parser.h"
#include "eclipse/scene/scene.h"
#include "eclipse/scene/scene_io.h"
#include "eclipse/render/options.h"
#include "eclipse/render/interactive_renderer.h"

#include <iostream>
#include <string>
#include <memory>

using namespace eclipse;

namespace {
    auto logger = Logger::create("main");
}

void show_usage()
{
    std::cout << "Eclipse - Path tracing renderer\n\n"
              << "usage: eclipse --help\n"
              << "usage: eclipse --info scene.(obj|bin)\n"
              << "usage: eclipse --compile scene.obj\n"
              << "usage: eclipse --list-devices\n"
              << "usage: eclipse --render scene.(obj|bin) [-w width] [-h height] [-spp spp]\n"
              << "                                        [-b num_bounces] [-rr bounces_before_RR]\n"
              << "                                        [-exp exposure]\n\n"
              << "options in order of precedence:\n"
              << "       --help         Print this menu\n"
              << "       --info         Print scene statistics\n"
              << "       --list-devices List the available rendering devices\n"
              << "       --compile      Compile scene to a compressed binary format\n"
              << "       --render       Render a scene" << std::endl;
}

int main(int argc, char** argv)
{
    try
    {
        InputParser input(argc, argv);

        if (input.option_exists("--help") || argc == 1)
        {
            show_usage();
        }
        else if (input.option_exists("--info"))
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
        else if (input.option_exists("--list-devices"))
        {
            logger.log<WARNING>("not implemented yet");
        }
        else if (input.option_exists("--compile"))
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
        else if (input.option_exists("--render"))
        {
            render::Options options;
            options.frame_width = 1024;
            options.frame_height = 1024;
            options.samples_per_pixel = 0;
            options.num_bounces = 5;
            options.min_bounces_for_rr = 3;
            options.exposure = 1.2f;

            if (input.option_exists("-w"))
                options.frame_width = std::stol(input.get_option("-w"));
            if (input.option_exists("-h"))
                options.frame_height = std::stol(input.get_option("-h"));
            if (input.option_exists("-spp"))
                options.samples_per_pixel = std::stof(input.get_option("-spp"));
            if (input.option_exists("-b"))
                options.num_bounces = std::stol(input.get_option("-b"));
            if (input.option_exists("-rr"))
                options.min_bounces_for_rr = std::stol(input.get_option("-rr"));
            if (input.option_exists("-exp"))
                options.exposure = std::stof(input.get_option("-exp"));

            if (options.num_bounces == 0 || options.min_bounces_for_rr >= options.num_bounces)
            {
                logger.log<INFO>("disabling russian roulette for path elimination");
                options.min_bounces_for_rr = options.num_bounces + 1;
            }

            std::string scene_file = input.get_option("--render");
            if (scene_file[0] == '-' || scene_file.empty())
                throw Error("missing scene file argument");

            std::shared_ptr<Resource> scene_res = std::make_shared<Resource>(scene_file);
            std::shared_ptr<scene::Scene> scene = scene::read(scene_res);

            return std::make_unique<render::InteractiveRenderer>(scene, options)->render();
        }
        else
        {
            logger.log<WARNING>("unknown option ", input.get_options()[0], "; use --help to list the available options");
        }
    }
    catch (std::exception& e)
    {
        logger.log<ERROR>("Exception caught: ", e.what());
    }

    return 0;
}
