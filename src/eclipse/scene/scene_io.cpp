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
    else if (has_extension(res->get_path(), ".bin"))
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

    std::ostringstream oss;
    oss << *scene;

    z_stream zs;
    std::memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK)
        throw Error("write: can't initialize zlib: " + std::string(zs.msg));

    zs.next_in = (Bytef*)oss.str().data();
    zs.avail_in = oss.str().size();

    int ret;
    char out_buffer[32768];
    std::string str_data;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(out_buffer);
        zs.avail_out = sizeof(out_buffer);

        ret = deflate(&zs, Z_FINISH);

        if (str_data.size() < zs.total_out)
            str_data.append(out_buffer, zs.total_out - str_data.size());

    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END)
        throw IOError("write: error during compression (" + std::to_string(ret) + "): " + zs.msg);

    std::ofstream out_file(file);
    out_file << str_data;
    out_file.close();

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
        throw Error("read_zip: can't initialize zlib: " + std::string(zs.msg));

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
    std::istringstream iss(str_data);
    iss >> *scene;

    stop_watch.stop();
    logger.log<INFO>("loaded scene in ", stop_watch.get_elapsed_time_ms(), " ms");

    return std::move(scene);
}

} } // namespace eclipse::scene
