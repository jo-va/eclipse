#include "eclipse/scene/scene_io.h"
#include "eclipse/scene/raw_scene.h"
#include "eclipse/scene/obj_loader.h"
#include "eclipse/scene/compiler.h"
#include "eclipse/util/resource.h"
#include "eclipse/util/file_util.h"
#include "eclipse/util/logger.h"
#include "eclipse/util/stop_watch.h"

#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <memory>
#include <utility>
#include <zlib.h>

namespace eclipse { namespace scene {

namespace {
    auto logger = Logger::create("scene_io");
}

std::unique_ptr<Scene> read_zip(std::shared_ptr<Resource> res);

std::unique_ptr<Scene> read(std::shared_ptr<Resource> res)
{
    if (has_extension(res->get_path(), ".obj"))
    {
        std::shared_ptr<raw::Scene> raw_scene = load_obj(res);
        std::unique_ptr<Scene> scene = compile(raw_scene);
        return std::move(scene);
    }
    else if (has_extension(res->get_path(), ".zip"))
    {
        std::unique_ptr<Scene> scene = read_zip(res);
        return std::move(scene);
    }
    else
    {
        throw IOError("read: unsupported file format");
    }
}

void write(std::shared_ptr<Scene> scene, const std::string& file)
{
    StopWatch stop_watch;
    stop_watch.start();
    logger.log<INFO>("compressing scene to " + file);

    if (!scene)
        logger.log<WARNING>("write: empty scene");

    create_dir(remove_filename(file));

    stop_watch.stop();
    logger.log<INFO>("compressed scene in ", stop_watch.get_elapsed_time_ms(), " ms");
}

std::unique_ptr<Scene> read_zip(std::shared_ptr<Resource> res)
{
    StopWatch stop_watch;
    stop_watch.start();
    logger.log<INFO>("parsing compiled scene from ", res->get_path());

    auto data = read_file(res->get_path());
    if (data.empty())
        throw IOError("read_zip: cant't open zip file " + res->get_path());

    z_stream zs;
    std::memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
        throw Error("can't initialize zlib");

    zs.next_in = (Bytef*)data.data();
    zs.avail_in = data.size();

    int ret;
    char out_buffer[32768];
    std::string str_data;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(out_buffer);
        zs.avail_out = sizeof(out_buffer);

        ret = inflate(&zs, 0);

        if (str_data.size() < zs.total_out)
            str_data.append(out_buffer, zs.total_out - str_data.size());

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END)
        throw IOError("read_zip: error during decompression (" + std::to_string(ret) + "): " + zs.msg);

    // deserialize scene from decompressed data
    auto scene = std::make_unique<Scene>();
    std::istringstream ss(str_data);
    ss >> *scene;

    stop_watch.stop();
    logger.log<INFO>("loaded scene in ", stop_watch.get_elapsed_time_ms(), " ms");

    return std::move(scene);
}

} } // namespace eclipse::scene
