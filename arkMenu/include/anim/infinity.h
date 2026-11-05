#ifndef INFINITY_ANIM_H
#define INFINITY_ANIM_H

#include "anim.h"

#define INFINITY_TRAIL_LENGTH 400
#define INFINITY_SPARKLE_COUNT 48

class InfinityAnim : public Anim {
    private:
        struct Point {
            float x;
            float y;
        };

        struct Sparkle {
            float x;
            float y;
            float vx;
            float vy;
            float age;
            float lifetime;
            float size;
        };

        float time;
        float emission;
        unsigned int lastTime;
        Point trail[INFINITY_TRAIL_LENGTH];
        Sparkle sparkles[INFINITY_SPARKLE_COUNT];

        void spawnSparkle(float x, float y);

    public:
        InfinityAnim();
        int getId();
        void draw();
};

#endif
