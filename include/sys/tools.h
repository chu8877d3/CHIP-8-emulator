#ifndef TOOLS_H
#define TOOLS_H

#include <nuklear.h>
#include <stdint.h>
#include <stdio.h>

static inline uint32_t color_to_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return (uint32_t)((((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (uint32_t)a));
}
static inline uint8_t color_get_r(uint32_t rgba)
{
    return (uint8_t)((rgba >> 24) & 0xff);
}
static inline uint8_t color_get_g(uint32_t rgba)
{
    return (uint8_t)((rgba >> 16) & 0xff);
}
static inline uint8_t color_get_b(uint32_t rgba)
{
    return (uint8_t)((rgba >> 8) & 0xff);
}
static inline uint8_t color_get_a(uint32_t rgba)
{
    return (uint8_t)((rgba) & 0xff);
}
static inline struct nk_color u32_to_nk(uint32_t rgba)
{
    return nk_rgba(color_get_r(rgba), color_get_g(rgba), color_get_b(rgba), color_get_a(rgba));
}
static inline uint32_t nk_to_u32(struct nk_color col)
{
    return color_to_u32(col.r, col.g, col.b, col.a);
}
static inline uint32_t nkf_to_u32(struct nk_colorf colf)
{
    struct nk_color col = nk_rgba_cf(colf);
    return nk_to_u32(col);
}
static inline void color_to_hex_str(struct nk_color col, char* buff)
{
    sprintf(buff, "#%02X%02X%02X", col.r, col.g, col.b);
}
static inline struct nk_color hex_str_to_color(const char* buff)
{
    struct nk_color col = { 0, 0, 0, 255 };
    unsigned int r, g, b;
    if (sscanf(buff + 1, "%02X%02X%02X", &r, &g, &b)) {
        col.r = r;
        col.g = g;
        col.b = b;
    }
    return col;
}
#endif