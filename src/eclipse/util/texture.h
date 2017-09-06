#pragma once

#include "eclipse/util/resource.h"
#include "eclipse/util/except.h"

#include <string>
#include <memory>
#include <cstdint>

namespace eclipse {

class TextureError : public Error
{
public:
    TextureError(const std::string& msg) : Error(msg) { }
};

class Texture
{
public:
    Texture(std::shared_ptr<Resource> res);
    ~Texture();

    enum Format
    {
        LUMINANCE8,
        LUMINANCE32F,
        RGBA8,
        RGBA32F
    };

    uint32_t get_width() const { return m_width; }
    uint32_t get_height() const { return m_height; }
    uint32_t get_size() const { return m_size; }
    Format get_format() const { return m_format; }
    uint8_t* get_data() const { return m_pixels; }

private:
    Format m_format;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_size;
    uint8_t* m_pixels;
};

} // namespace eclipse
