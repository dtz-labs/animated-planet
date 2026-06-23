#include "video.h"

#include <intrinsic.h>
#include <string.h>
#include <z80.h>

#define TIMEX_PORT      0x00FFu
#define TIMEX_PAGE_A    0x00u
#define TIMEX_PAGE_B    0x01u

#ifdef ZX128_PAGE_FLIP
extern void zx128_page_show_a(void);
extern void zx128_page_show_b(void);
#endif

static u8 front_page;
u16 video_row_off[SCREEN_H];

static void build_row_offsets(void)
{
    u8 y;

    for (y = 0; y < SCREEN_H; y++) {
        video_row_off[y] = (u16)(((u16)(y & 0xC0u) << 5)
            + ((u16)(y & 0x07u) << 8)
            + ((u16)(y & 0x38u) << 2));
    }
}

static void fill_attrs(u16 base, u8 tick)
{
    static const u8 inks[8] = { 7, 6, 4, 5, 3, 2, 6, 7 };
    u8 *attrs = (u8 *)(uintptr_t)base;
    u16 i = 0;
    u8 row;

    for (row = 0; row < 24u; row++) {
        u8 col;
        for (col = 0; col < 32u; col++, i++) {
            u8 band = (u8)((row + (col >> 2) + tick) & 7u);
            attrs[i] = ATTR(1, 0, inks[band]);
        }
    }
}

void video_clear(u16 base)
{
    memset((u8 *)(uintptr_t)base, 0, BITMAP_LEN);
}

void video_paint_attrs(u8 tick)
{
    fill_attrs(ATTRS_A, tick);
    fill_attrs(ATTRS_B, tick);
}

void video_init(void)
{
    intrinsic_im_1();
    intrinsic_ei();
    build_row_offsets();

#ifdef ZX128_PAGE_FLIP
    zx128_page_show_a();
#endif
    video_clear(SCREEN_A);
    video_clear(SCREEN_B);
    video_paint_attrs(0);

    front_page = 0;
#ifndef ZX128_PAGE_FLIP
    z80_outp(TIMEX_PORT, TIMEX_PAGE_A);
#endif
}

void video_show_a(void)
{
    front_page = 0;
#ifdef ZX128_PAGE_FLIP
    zx128_page_show_a();
#else
    z80_outp(TIMEX_PORT, TIMEX_PAGE_A);
#endif
}

u16 video_back(void)
{
    return front_page ? SCREEN_A : SCREEN_B;
}

u8 video_back_page(void)
{
    return (u8)(front_page ^ 1u);
}

void video_present(void)
{
    intrinsic_halt();
    front_page ^= 1u;
#ifdef ZX128_PAGE_FLIP
    if (front_page) {
        zx128_page_show_b();
    } else {
        zx128_page_show_a();
    }
#else
    z80_outp(TIMEX_PORT, front_page ? TIMEX_PAGE_B : TIMEX_PAGE_A);
#endif
}

void video_wait(void)
{
    intrinsic_halt();
}
