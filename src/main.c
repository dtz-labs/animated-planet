#include "globe.h"
#include "types.h"
#include "video.h"

#include <string.h>
#include <z80.h>

#define PLANET_CX   128u
#define PLANET_CY    96u
#define PLANET_R     70u

static u8 touched_n[2];
static u16 touched_off[2][GLOBE_MAXPTS];
static u16 planet_row_off[GLOBE_MAXPTS];
static u8 planet_count;

static void reset_trails(void)
{
    touched_n[0] = 0;
    touched_n[1] = 0;
}

static void init_planet(void)
{
    u8 i;

    globe_init(PLANET_CX, PLANET_CY, PLANET_R);
    planet_count = globe_count();
    for (i = 0; i < planet_count; i++) {
        planet_row_off[i] = video_row_off[globe_y(i)];
    }
    reset_trails();
}

static void paint_space_attrs(void)
{
    u8 *a = (u8 *)ATTRS_A;
    u8 *b = (u8 *)ATTRS_B;
    u16 i;

    for (i = 0; i < ATTRS_LEN; i++) {
        u8 row = (u8)(i >> 5);
        u8 col = (u8)(i & 31u);
        u8 attr = ATTR(0, 0, 7);

        if (row > 3u && row < 20u && col > 5u && col < 26u) {
            attr = ATTR(1, 0, 5);  /* bright cyan planet wireframe area */
        }
        if (((row * 11u + col * 7u) & 31u) == 0u) {
            attr = ATTR(1, 0, 7);  /* sparse star cells */
        }
        a[i] = attr;
        b[i] = attr;
    }
}

static void plot_point(u16 base, u8 x, u8 y)
{
    ((u8 *)(uintptr_t)(base + video_row_off[y]))[x >> 3] |= (u8)(0x80u >> (x & 7u));
}

static void plot_starfield(u16 base)
{
    static const u8 stars[][2] = {
        { 10, 14 }, { 42, 42 }, { 76, 12 }, { 211, 27 },
        { 238, 77 }, { 19, 141 }, { 55, 172 }, { 202, 158 },
        { 232, 181 }, { 121, 21 }, { 145, 168 }, { 91, 183 }
    };
    u8 i;

    for (i = 0; i < sizeof(stars) / sizeof(stars[0]); i++) {
        plot_point(base, stars[i][0], stars[i][1]);
    }
}

static void draw_planet(u8 theta)
{
    u16 base = video_back();
    u8 *screen = (u8 *)(uintptr_t)base;
    u8 page = video_back_page();
    u8 i;
    u8 n = 0;

    for (i = 0; i < touched_n[page]; i++) {
        screen[touched_off[page][i]] = 0u;
    }

    plot_starfield(base);

    for (i = 0; i < planet_count; i++) {
        u8 x;
        if (globe_project_front_x(i, theta, &x)) {
            u8 col = (u8)(x >> 3);
            u16 off = (u16)(planet_row_off[i] + col);
            screen[off] |= (u8)(0x80u >> (x & 7u));
            touched_off[page][n++] = off;
        }
    }
    touched_n[page] = n;
}

int main(void)
{
    u8 theta = 0;

    video_init();
    video_clear(SCREEN_A);
    video_clear(SCREEN_B);
    paint_space_attrs();
    plot_starfield(SCREEN_A);
    plot_starfield(SCREEN_B);

    init_planet();

    for (;;) {
        draw_planet(theta);
        video_present();
        theta = (u8)(theta + 2u);
        z80_outp(0x00FEu, 0u);
    }
}
