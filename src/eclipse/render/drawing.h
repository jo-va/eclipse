#pragma once

#include "eclipse/renderer/default_font.h"
#include <cstdint>
#include <cmath>

namespace draw {

template <typename T>
struct Point
{
    T x;
    T y;

    Point(T x, T y) : x(x), y(y) { }
};

template <typename T>
struct Buffer
{
    T* buffer;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
};

template <typename T>
void clear(const Buffer<T>& buffer, Point<uint32_t> p1, Point<uint32_t> p2, T color)
{
    if (!buffer.buffer) return;
    for (uint32_t i = p1.y; i < p2.y; ++i)
        for (uint32_t j = p1.x; j < p2.x; ++j)
            buffer.buffer[i * buffer.pitch + j] = color;
}

template <typename T>
void line(const Buffer<T>& buffer, Point<float> p1, Point<float> p2, T color)
{
    if (!buffer.buffer) return;
    if (p1.x < 0) p1.x = 0;
    if (p2.x < 0) p2.x = 0;
    if (p1.y < 0) p1.y = 0;
    if (p2.y < 0) p2.y = 0;
    if (p1.x >= buffer.width) p1.x = buffer.width;
    if (p2.x >= buffer.width) p2.x = buffer.width;
    if (p1.y >= buffer.height) p1.y = buffer.height;
    if (p2.y >= buffer.height) p2.y = buffer.height;

    float w = p2.x - p1.x;
    float h = p2.y - p1.y;
    float l = std::abs(w);
    if (std::abs(h) > l) l = std::abs(h);
    int il = (int)l;
    float dx = w / l;
    float dy = h / l;

    for (int i = 0; i <= il; ++i) {
        buffer.buffer[(int)p1.x + (int)p1.y * buffer.pitch] = color;
        p1.x += dx;
        p1.y += dy;
    }
}

template <typename T>
void box(const Buffer<T>& buffer, Point<uint32_t> p1, Point<uint32_t> p2, T color)
{
    if (!buffer.buffer) return;
    line(buffer, { (float)p1.x, (float)p1.y }, { (float)p2.x, (float)p1.y }, color);
    line(buffer, { (float)p2.x, (float)p1.y }, { (float)p2.x, (float)p2.y }, color);
    line(buffer, { (float)p1.x, (float)p2.y }, { (float)p2.x, (float)p2.y }, color);
    line(buffer, { (float)p1.x, (float)p1.y }, { (float)p1.x, (float)p2.y }, color);
}

template <typename T>
void bar(const Buffer<T>& buffer, Point<uint32_t> p1, Point<uint32_t> p2, T color)
{
    if (!buffer.buffer) return;
    if (p1.x > buffer.width) p1.x = buffer.width;
    if (p2.x > buffer.width) p2.x = buffer.width;
    if (p1.y > buffer.height) p1.y = buffer.height;
    if (p2.y > buffer.height) p2.y = buffer.height;

    T* ptr = buffer.buffer + p1.y * buffer.pitch + p1.x;

    for (uint32_t y = p1.y; y < p2.y; ++y) {
        for (uint32_t x = 0; x < (p2.x - p1.x); ++x)
            ptr[x] = color;
        ptr += buffer.pitch;
    }
}

template <typename T>
void print(const Buffer<T>& buffer, const char* str, Point<uint32_t> pos, T color)
{
    if (!buffer.buffer)
        return;
    if (pos.y + (uint32_t)FONT_HEIGHT > buffer.height)
        return;

    while (*str != '\0') {

        if (pos.x + (uint32_t)FONT_WIDTH > buffer.width)
            return;

        uint32_t idx = pos.y * buffer.pitch + pos.x;
        const uint8_t * const font = default_font[(uint8_t)*str];

        for (uint32_t h = 0; h < FONT_HEIGHT; ++h) {
            for (uint32_t w = 0; w < FONT_WIDTH; ++w) {
                if (font[FONT_HEIGHT - h - 1] & (1 << w))
                    buffer.buffer[idx] = color;
                ++idx;
            }
            idx += buffer.pitch - FONT_WIDTH;
        }
        pos.x += FONT_WIDTH + FONT_SPACE;
        ++str;
    }
}

} // namespace draw
