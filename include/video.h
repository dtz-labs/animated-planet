#ifndef VIDEO_H
#define VIDEO_H

#include "types.h"

#define SCREEN_A       0x4000u
#define ATTRS_A        0x5800u

#ifdef ZX128_PAGE_FLIP
#define SCREEN_B       0xC000u
#define ATTRS_B        0xD800u
#else
#define SCREEN_B       0x6000u
#define ATTRS_B        0x7800u
#endif

#define BITMAP_LEN     6144u
#define ATTRS_LEN      768u
#define SCREEN_W       256u
#define SCREEN_H       192u
#define ROW_BYTES      32u

#define ATTR(bright, paper, ink) \
    ((u8)((((bright) & 1u) << 6) | (((paper) & 7u) << 3) | ((ink) & 7u)))

extern u16 video_row_off[SCREEN_H];

static inline u8 *video_scanline(u16 base, u8 y)
{
    return (u8 *)(uintptr_t)(base + video_row_off[y]);
}

void video_init(void);
void video_show_a(void);
void video_present(void);
void video_wait(void);
u16  video_back(void);
u8   video_back_page(void);
void video_clear(u16 base);
void video_paint_attrs(u8 tick);

#endif
