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

void write(std::shared_ptr<Scene> scene, std::shared_ptr<Resource> res)
{
    std::string filename = remove_extension(res->get_path()) + ".bin";
    StopWatch stop_watch;
    stop_watch.start();
    logger.log<INFO>("compressing scene to " + filename);

    if (!scene)
        logger.log<WARNING>("write: empty scene");

    create_dir(remove_filename(filename));

    std::ostringstream oss;
    scene->serialize(oss);

    size_t ucomp_size = oss.str().size();
    size_t comp_size = compressBound(ucomp_size);

    std::vector<char> buffer(comp_size);
    compress((Bytef*)&buffer[0], &comp_size, (Bytef*)oss.str().c_str(), ucomp_size);
    buffer.resize(comp_size);

    std::ofstream out_file(filename);
    out_file.write((char*)&ucomp_size, sizeof(ucomp_size));
    out_file.write((char*)&buffer[0], buffer.size());
    out_file.close();

    stop_watch.stop();
    logger.log<INFO>("compressed scene in ", stop_watch.get_elapsed_time_ms(), " ms");
}

std::unique_ptr<Scene> read_zip(std::shared_ptr<Resource> res)
{
    StopWatch stop_watch;
    stop_watch.start();
    logger.log<INFO>("parsing compiled scene from ", res->get_path());

    std::ifstream in_file(res->get_path());
    in_file.seekg(0, std::ios::end);
    uLong size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);

    size_t ucomp_size;
    in_file.read((char*)&ucomp_size, sizeof(ucomp_size));

    size_t data_size = size - sizeof(ucomp_size);
    std::vector<char> data(data_size);
    in_file.read(&data[0], data_size);

    std::string buffer;
    buffer.resize(ucomp_size);
    uncompress((Bytef*)buffer.data(), &ucomp_size, (Bytef*)&data[0], data.size());

    // deserialize scene from decompressed data
    auto scene = std::make_unique<Scene>();
    std::istringstream iss(buffer);
    scene->deserialize(iss);

    stop_watch.stop();
    logger.log<INFO>("loaded scene in ", stop_watch.get_elapsed_time_ms(), " ms");

    return std::move(scene);
}

} } // namespace eclipse::scene
