#include "globe.h"

#include <stdio.h>

static int failures = 0;

static void check(const char *name, int cond)
{
    if (!cond) {
        printf("FAIL %s\n", name);
        failures++;
    }
}

int main(void)
{
    u8 count;
    int theta;
    int moved = 0;

    globe_init(128u, 96u, 70u);
    count = globe_count();

    check("point count is stable", count == 153u);
    check("point count fits caller storage", count <= GLOBE_MAXPTS);

    for (theta = 0; theta < 256; theta += 17) {
        int front_count = 0;
        u8 i;

        for (i = 0; i < count; i++) {
            u8 x = globe_x(i, (u8)theta);
            u8 y = globe_y(i);
            u8 projected_x = 0u;
            u8 projected_y = 0u;
            u8 projected_front = globe_project_front(i, (u8)theta, &projected_x, &projected_y);
            u8 projected_front_x = globe_project_front_x(i, (u8)theta, &projected_x);

            check("x stays inside planet bounds", x >= 58u && x <= 198u);
            check("y stays inside planet bounds", y >= 26u && y <= 166u);
            check("front helpers agree", projected_front == globe_front(i, (u8)theta));
            check("front-x helper agrees", projected_front_x == projected_front);

            if (projected_front) {
                front_count++;
                check("projected x matches globe_x", projected_x == x);
                check("projected y matches globe_y", projected_y == y);
            }

            if (globe_x(i, 0u) != globe_x(i, 64u)) {
                moved = 1;
            }
        }

        check("rotation has visible front points", front_count > 0);
    }

    check("rotation changes projected x positions", moved);

    if (failures == 0) {
        printf("test_globe: ALL PASS\n");
        return 0;
    }

    printf("test_globe: %d FAILURE(S)\n", failures);
    return 1;
}
