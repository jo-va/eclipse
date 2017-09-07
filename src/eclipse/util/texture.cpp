#include "eclipse/util/texture.h"

#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include <cstdint>
#include <OpenImageIO/imageio.h>
OIIO_NAMESPACE_USING

namespace eclipse {

Texture::Texture(std::shared_ptr<Resource> res)
    : m_size(0), m_pixels(nullptr)
{
    ImageInput* input = ImageInput::open(res->get_path());
    if (!input)
        throw TextureError("Can't open texture " + res->get_path() + ": " + OpenImageIO::geterror());

    const ImageSpec& spec = input->spec();
    m_width = spec.width;
    m_height = spec.height;

    if (spec.nchannels != 1 && spec.nchannels != 3 && spec.nchannels != 4)
    {
        input->close();
        throw TextureError("texture: unsupported channel count " + std::to_string(spec.nchannels) +
                           " while loading " + res->get_path());
    }

    if (spec.depth != 1)
    {
        input->close();
        throw TextureError("texture: unsupported depth " + std::to_string(spec.depth) +
                           " while loading " + res->get_path());
    }

    std::vector<uint8_t> data;
    TypeDesc convert_to;
    if (spec.format == TypeDesc::UINT8)
    {
        data.resize(m_width * m_height * spec.nchannels);

        convert_to = TypeDesc::UINT8;
        if (spec.nchannels == 1)
            m_format = LUMINANCE8;
        else
            m_format = RGBA8;
    }
    else
    {
        data.resize(m_width * m_height * spec.nchannels * sizeof(float));

        convert_to = TypeDesc::FLOAT;
        if (spec.nchannels == 1)
            m_format = LUMINANCE32F;
        else
            m_format = RGBA32F;
    }

    if (!input->read_image(convert_to, (void*)data.data(), AutoStride, AutoStride, AutoStride))
    {
        std::string error = input->geterror();
        input->close();
        throw TextureError("texture: could not read pixels from " + res->get_path() + ": " + error);
    }

    m_size = data.size();

    // convert to RGBA as this makes addressing in opencl much easier
    if (convert_to == TypeDesc::UINT8 && spec.nchannels == 3)
    {
        uint8_t* pixels = new uint8_t[m_width * m_height * 4];
        uint32_t w_offset = 0, r_offset = 0;
        while (r_offset < data.size())
        {
            pixels[w_offset + 0] = data[r_offset + 0];
            pixels[w_offset + 1] = data[r_offset + 1];
            pixels[w_offset + 2] = data[r_offset + 2];
            pixels[w_offset + 3] = 255;

            r_offset += 3;
            w_offset += 4;
        }
        m_pixels = pixels;
    }
    else if (convert_to == TypeDesc::FLOAT && spec.nchannels == 3)
    {
        float* pixels = new float[m_width * m_height * 4];
        uint32_t w_offset = 0, r_offset = 0;
        float* data_float = (float*)data.data();
        while (r_offset < data.size() / sizeof(float))
        {
            pixels[w_offset + 0] = data_float[r_offset + 0];
            pixels[w_offset + 1] = data_float[r_offset + 1];
            pixels[w_offset + 2] = data_float[r_offset + 2];
            pixels[w_offset + 3] = 1.0f;

            r_offset += 3;
            w_offset += 4;
        }
        m_pixels = (uint8_t*)pixels;
    }
    else
    {
        m_pixels = new uint8_t[data.size()];
        std::memcpy(m_pixels, data.data(), data.size());
    }

    if (!input->close())
    {
        throw TextureError("texture: could not close " + res->get_path());
    }
}

Texture::~Texture()
{
    if (m_pixels)
        delete[] m_pixels;
    m_pixels = nullptr;
}

} // namespace eclipse
