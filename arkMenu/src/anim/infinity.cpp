/*
 * Infinity background animation, adapted from infinityintro.cpp.
 *
 * Copyright (C) 2015, David "Davee" Morgan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <cmath>
#include <cstdlib>

#include "infinity.h"
#include "animtypes.h"
#include "common.h"

InfinityAnim::InfinityAnim() : time(0.f), emission(0.f),
    lastTime(sceKernelGetSystemTimeLow())
{
    for (int i = 0; i < INFINITY_TRAIL_LENGTH; ++i) {
        trail[i].x = 457.5f;
        trail[i].y = 126.f;
    }

    for (int i = 0; i < INFINITY_SPARKLE_COUNT; ++i)
        sparkles[i].age = sparkles[i].lifetime = 0.f;
}

int InfinityAnim::getId()
{
    return ANIM_INFINITY;
}

void InfinityAnim::spawnSparkle(float x, float y)
{
    for (int i = 0; i < INFINITY_SPARKLE_COUNT; ++i) {
        Sparkle& sparkle = sparkles[i];
        if (sparkle.age < sparkle.lifetime)
            continue;

        const float angle = (float)(rand() % 6284) / 1000.f;
        const float speed = 15.f + (float)(rand() % 400) / 10.f;
        sparkle.x = x + (float)(rand() % 11 - 5);
        sparkle.y = y + (float)(rand() % 11 - 5);
        sparkle.vx = cosf(angle) * speed;
        sparkle.vy = sinf(angle) * speed;
        sparkle.age = 0.f;
        sparkle.lifetime = 0.35f + (float)(rand() % 35) / 100.f;
        sparkle.size = 1.f + (float)(rand() % 20) / 10.f;
        return;
    }
}

void InfinityAnim::draw()
{
    const u32 now = sceKernelGetSystemTimeLow();
    float dt = (float)(now - lastTime) / 1000000.f;
    lastTime = now;
    if (dt > 0.1f)
        dt = 0.1f;

    time += dt;
    const float scale = 2.f / (3.f - cosf(2.f * time));
    const float modelX = scale * cosf(time);
    const float modelY = scale * sinf(2.f * time) / 2.f;
    const float x = 440.f * ((modelX + 1.f) / 2.f) + 17.5f;
    const float y = 272.f * ((modelY + 1.f) / 2.f) - 10.f;

    for (int i = INFINITY_TRAIL_LENGTH - 1; i > 0; --i)
        trail[i] = trail[i - 1];
    trail[0].x = x;
    trail[0].y = y;

    emission += dt * 60.f;
    while (emission >= 1.f) {
        spawnSparkle(x, y);
        emission -= 1.f;
    }

    for (int i = 0; i < INFINITY_TRAIL_LENGTH - 1; ++i) {
        const int alpha = 40 + (215 * (INFINITY_TRAIL_LENGTH - i)) /
                                  INFINITY_TRAIL_LENGTH;
        const u32 glow = 0x00FFFFFF | ((alpha / 5) << 24);
        const u32 color = 0x00FFFFFF | (alpha << 24);
        ya2d_draw_line(trail[i].x - 2.f, trail[i].y, trail[i + 1].x - 2.f,
                       trail[i + 1].y, glow);
        ya2d_draw_line(trail[i].x + 2.f, trail[i].y, trail[i + 1].x + 2.f,
                       trail[i + 1].y, glow);
        ya2d_draw_line(trail[i].x, trail[i].y - 2.f, trail[i + 1].x,
                       trail[i + 1].y - 2.f, glow);
        ya2d_draw_line(trail[i].x, trail[i].y + 2.f, trail[i + 1].x,
                       trail[i + 1].y + 2.f, glow);
        ya2d_draw_line(trail[i].x, trail[i].y, trail[i + 1].x,
                       trail[i + 1].y, color);
    }

    for (int i = 0; i < INFINITY_SPARKLE_COUNT; ++i) {
        Sparkle& sparkle = sparkles[i];
        if (sparkle.age >= sparkle.lifetime)
            continue;

        sparkle.age += dt;
        sparkle.x += sparkle.vx * dt;
        sparkle.y += sparkle.vy * dt;

        float opacity = 1.f - sparkle.age / sparkle.lifetime;
        if (opacity < 0.f)
            opacity = 0.f;
        const int alpha = (int)(opacity * 190.f);
        const int size = (int)sparkle.size;
        const u32 glow = 0x00FFFFFF | ((alpha / 4) << 24);
        const u32 color = 0x00FFFFFF | (alpha << 24);
        ya2d_draw_rect(sparkle.x - size - 2, sparkle.y - size - 2,
                       size * 2 + 5, size * 2 + 5, glow, 1);
        ya2d_draw_rect(sparkle.x - size, sparkle.y - size, size * 2 + 1,
                       size * 2 + 1, color, 1);
    }
}
