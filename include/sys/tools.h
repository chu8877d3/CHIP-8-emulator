#ifndef TOOLS_H
#define TOOLS_H

#include <stdint.h>
#define MAKE_RGBA(r, g, b, a) (((r) << 24) | ((g) << 16) | ((b) << 8) | a)

static inline uint32_t make_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return (uint32_t)(((r << 24) | (g << 16) | (b << 8) | a));
}
static inline uint8_t r(uint32_t rgba)
{
    return (uint8_t)((rgba >> 24) & 0xff);
}

static inline uint8_t g(uint32_t rgba)
{
    return (uint8_t)((rgba >> 16) & 0xff);
}
static inline uint8_t b(uint32_t rgba)
{
    return (uint8_t)((rgba >> 8) & 0xff);
}
static inline uint8_t a(uint32_t rgba)
{
    return (uint8_t)((rgba) & 0xff);
}

#endif