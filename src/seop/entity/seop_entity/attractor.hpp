#pragma once
#include "seop_math/math.hpp"

namespace seop::entity
{
    struct Attractor
    {
        math::Vec4 pos{200.0f,200.0f, 200.0f, 1.0f}; // 위치
        float str{100.0f};            // 힘
        float rad{50.0f};            // 거리
        float padding[2]{0.0f, 0.0f};
    };

}
