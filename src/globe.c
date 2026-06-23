/*
 * globe.c -- see globe.h. A wireframe globe: NMER meridian arcs (vertical) and
 * NPAR parallel rings (horizontal), projected with a fixed-point Y-axis
 * rotation. X offsets are precomputed once per distinct projected latitude
 * radius so the frame loop avoids fixed-point multiplies.
 */
#include "globe.h"
#include "fxtab.h"

#define NMER 9u     /* meridian lines (longitudes)                    */
#define MARC 9u     /* dots per meridian (dense enough to read motion)*/
#define MARC_LO  (-72)  /* meridian latitude range -72..72            */
#define MARC_STEP 18    /* 144 / (MARC-1) deg between meridian dots    */
#define NPAR 6u     /* parallel rings (latitudes)                     */
#define PARC 12u    /* dots per parallel                              */
#define PAR_LO  (-60)   /* parallel latitudes -60..60 (skip the poles)*/
#define PAR_STEP 24     /* 120 / (NPAR-1) deg between parallels        */
#define NPTS (NMER * MARC + NPAR * PARC)   /* 81 + 72 = 153 */
#define MAX_RADII 8u

static u8 g_cx, g_cy;
static u8 g_ridx[NPTS];   /* index into g_xoff for this latitude */
static u8 g_sy[NPTS];     /* screen y (constant per point)      */
static u8 g_lon[NPTS];    /* base longitude index (0..255)      */
static u8 g_nradii;
static u8 g_rval[MAX_RADII];
static s8 g_xoff[MAX_RADII][256];

/* latitude angle (deg, -90..90) -> 0..255 phase index. */
static u8 lat_idx(s16 deg)
{
    return (u8)((s16)(deg * 256) / 360);
}

static u8 radius_index(u8 r)
{
    u8 i;

    for (i = 0; i < g_nradii; i++) {
        if (g_rval[i] == r) {
            return i;
        }
    }

    if (g_nradii < MAX_RADII) {
        u8 a;
        i = g_nradii++;
        g_rval[i] = r;
        for (a = 0; a != 0xffu; a++) {
            g_xoff[i][a] = (s8)fx_mul(fx_sin[(u8)(a + 64u)], r);
        }
        g_xoff[i][255] = (s8)fx_mul(fx_sin[63], r);
        return i;
    }

    return 0u;
}

void globe_init(u8 cx, u8 cy, u8 r)
{
    u8 m, k, p, j;
    u8 i = 0u;

    g_cx = cx;
    g_cy = cy;
    g_nradii = 0;

    /* meridian arcs: NMER longitudes, each a dense -78..78 latitude line */
    for (m = 0; m < NMER; m++) {
        u8 lon = (u8)(m * (256u / NMER));
        for (k = 0; k < MARC; k++) {
            s16 deg    = (s16)(MARC_LO + MARC_STEP * (s16)k);
            u8  a      = lat_idx(deg);
            s8  sinphi = fx_sin[a];
            s8  cosphi = fx_sin[(u8)(a + 64u)];
            g_ridx[i] = radius_index((u8)fx_mul(cosphi, r));
            g_sy[i]   = (u8)((s16)cy - fx_mul(sinphi, r));
            g_lon[i]  = lon;
            i++;
        }
    }

    /* parallel rings: NPAR latitudes, each a dense ring of PARC longitudes */
    for (p = 0; p < NPAR; p++) {
        s16 deg    = (s16)(PAR_LO + PAR_STEP * (s16)p);
        u8  a      = lat_idx(deg);
        s8  sinphi = fx_sin[a];
        s8  cosphi = fx_sin[(u8)(a + 64u)];
        u8  rp     = (u8)fx_mul(cosphi, r);
        u8  sy     = (u8)((s16)cy - fx_mul(sinphi, r));
        for (j = 0; j < PARC; j++) {
            g_ridx[i] = radius_index(rp);
            g_sy[i]   = sy;
            g_lon[i]  = (u8)(j * (256u / PARC));
            i++;
        }
    }
}

u8 globe_count(void) { return NPTS; }

u8 globe_x(u8 i, u8 theta)
{
    u8 a    = (u8)(g_lon[i] + theta);
    return (u8)((s16)g_cx + g_xoff[g_ridx[i]][a]);
}

u8 globe_y(u8 i) { return g_sy[i]; }

u8 globe_front(u8 i, u8 theta)
{
    u8 a = (u8)(g_lon[i] + theta);
    return (fx_sin[a] >= 0) ? 1u : 0u;
}

u8 globe_project_front(u8 i, u8 theta, u8 *x, u8 *y)
{
    u8 a = (u8)(g_lon[i] + theta);

    if (fx_sin[a] < 0) {
        return 0u;
    }

    *x = (u8)((s16)g_cx + g_xoff[g_ridx[i]][a]);
    *y = g_sy[i];
    return 1u;
}

u8 globe_project_front_x(u8 i, u8 theta, u8 *x)
{
    u8 a = (u8)(g_lon[i] + theta);

    if (fx_sin[a] < 0) {
        return 0u;
    }

    *x = (u8)((s16)g_cx + g_xoff[g_ridx[i]][a]);
    return 1u;
}
