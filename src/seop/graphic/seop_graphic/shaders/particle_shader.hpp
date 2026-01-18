#pragma once

namespace seop::graphic
{
const char* particle_vs_source = R"(
#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;
out vec4 vColor;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position =projection* view * aPos;
                        
    vColor = aColor; 
}
)";

const char* particle_fs_source = R"(    
#version 430 core
in vec4 vColor;
out vec4 FragColor;
void main() {
float dist = length(gl_PointCoord - vec2(0.5));
    
    // 중심은 밝고 외곽은 투명하게 (Glow 효과)
    float glow = exp(-dist * 2.0); // 숫자가 클수록 중심에 집중됨
    
    if (dist > 0.5) discard; // 원 밖은 버림

    FragColor = vec4(vColor.rgb, vColor.a * glow);
}
)";
} // namespace seop::graphic