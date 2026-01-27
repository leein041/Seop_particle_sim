#version 430 core
in vec4 worldPos;
out vec4 FragColor;

void main() {
// 1. 기존 격자 계산
    float grid_size = 100.0;
    vec2 grid = abs(fract(worldPos.xz / grid_size - 0.5) - 0.5) / fwidth(worldPos.xz / grid_size);
    float line = min(grid.x, grid.y);
    float grid_alpha = 1.0 - min(line, 1.0);

    float sub_grid_size = 50.0;
    vec2 sub_grid = abs(fract(worldPos.xz / sub_grid_size - 0.5) - 0.5) / fwidth(worldPos.xz / sub_grid_size);
    float sub_line = min(sub_grid.x, sub_grid.y);
    float sub_grid_alpha = 1.0 - min(sub_line, 1.0);

    // 2. 거리 계산 (원점으로부터의 거리)
    float dist = length(worldPos.xz);
    
    // 3. 가시 범위 설정
    float max_dist = 3000.0; // 격자가 보일 최대 반경
    float falloff = 1000.0;  // 서서히 사라지기 시작할 구간의 폭
    
    // 거리에 따른 투명도 감쇄 (0.0 ~ 1.0)
    float distance_alpha = 1.0 - smoothstep(max_dist - falloff, max_dist, dist);

    // 4. 최종 색상 결정
    vec4 color = vec4(0.5, 0.5, 0.5, grid_alpha * distance_alpha);
    vec4 sub_color = vec4(0.5, 0.5, 0.5, sub_grid_alpha * 0.5 * distance_alpha);

    // 축 강조 로직 (여기도 거리 감쇄 적용) 
    if (abs(worldPos.x) < 0.1) color = vec4(0.0, 0.0, 1.0, distance_alpha);
    if (abs(worldPos.z) < 0.1) color = vec4(1.0, 0.0, 0.0, distance_alpha);


    // 5. 투명도 체크 및 출력
    FragColor = mix(sub_color,color,grid_alpha);

    if (FragColor.a < 0.01) discard; // 너무 투명하면 아예 그리지 않음
}