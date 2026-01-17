#pragma once
namespace seop::graphic
{
const char* grid_vs_source = R"(
#version 430 core


layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;
uniform mat4 view;
uniform mat4 projection;
out vec4 worldPos;

void main() {
    worldPos = vec4(aPos.xyz * 100000.0, 1.0);
    gl_Position = projection * view * worldPos ;
}
)";
const char *grid_fs_source = R"(
#version 430 core
in vec4 worldPos;
out vec4 FragColor;

void main() {
// 1. 기존 격자 계산
    float grid_size = 100.0;
    vec2 grid = abs(fract(worldPos.xz / grid_size - 0.5) - 0.5) / fwidth(worldPos.xz / grid_size);
    float line = min(grid.x, grid.y);
    float grid_alpha = 1.0 - min(line, 1.0);

    // 2. 거리 계산 (원점으로부터의 거리)
    float dist = length(worldPos.xz);
    
    // 3. 가시 범위 설정
    float max_dist = 10000.0; // 격자가 보일 최대 반경
    float falloff = 100.0;  // 서서히 사라지기 시작할 구간의 폭
    
    // 거리에 따른 투명도 감쇄 (0.0 ~ 1.0)
    float distance_alpha = 1.0 - smoothstep(max_dist - falloff, max_dist, dist);

    // 4. 최종 색상 결정
    vec4 color = vec4(0.2, 0.2, 0.2, grid_alpha * distance_alpha);

    // 축 강조 로직 (여기도 거리 감쇄 적용)
    if (abs(worldPos.x) < 0.1) color = vec4(0.0, 0.0, 1.0, distance_alpha);
    if (abs(worldPos.z) < 0.1) color = vec4(1.0, 0.0, 0.0, distance_alpha);


    // 5. 투명도 체크 및 출력
    if (color.a < 0.01) discard; // 너무 투명하면 아예 그리지 않음
    FragColor = color;
}
)";
const char *particle_vs_source = R"(
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

// 2. Fragment Shader: "이 색으로 칠해라"
const char *particle_fs_source = R"(    
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

} // namespace