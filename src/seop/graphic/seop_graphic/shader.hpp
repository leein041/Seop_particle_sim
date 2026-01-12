namespace
{
const char *gravity_cs_source =
    R"(#version 430 core
        layout(local_size_x = 256) in; // 워크그룹 크기

    struct Particle
    {
        vec4 pos;
        vec4 col;
        vec4 vel;
    };
    struct Attractor
    {
        vec4 pos;
        float str;
        float rad;
        float padding[2];
    };
    layout(std430, binding = 0) buffer ParticleBuffer
    {
        Particle particles[];
    };

    layout(std430, binding = 1) buffer AttractorBuffer
    {
        Attractor attractors[];
    };

    uniform float u_dt;
    uniform float u_gravity;
    uniform float u_orbit_force;
    uniform float u_damping;
    uniform float u_magentic_str;
    uniform float u_particle_col;
    uniform uint u_attractor_count;
    uniform uint u_particle_count;
    uniform vec3 u_ray_point;

    void main()
    {
            // index
            uint i = gl_GlobalInvocationID.x; 
            if (i >= u_particle_count) return;

            // setting
            vec3 pos = particles[i].pos.xyz;
            vec3 vel = particles[i].vel.xyz;
            vec3 total_accel = vec3(0.0);

            // cursor
            float dis = length(u_ray_point - pos);
            dis = max(dis, 5.0);
            vec3 dir = (u_ray_point - pos) / dis;

            vec3 up = vec3(1.0, 2.0, 0.0);
            vec3 tangent_dir = normalize(cross(dir,up));

            // gravity
            total_accel += dir * u_gravity / (dis * dis);

            // vortex
            total_accel += tangent_dir * u_orbit_force / (dis * dis);
           
            // attractor
            
            // compute
            vel = (vel + total_accel * u_dt) * u_damping;
            pos = pos + vel * u_dt;

            // color
            float speed = length(vel);
            float t = speed * 0.005;
            float intensity = clamp(t, 0.0, 1.0);
            particles[i].col = vec4(intensity, u_particle_col, 1.0 - intensity, 1.0);

            // save
            particles[i].pos.xyz = pos;
            particles[i].vel.xyz = vel;
    }
)";
const char *ampere_cs_source = // B = I / 2πr
    R"(#version 430 core
        layout(local_size_x = 256) in; // 워크그룹 크기

    struct Particle
    {
        vec2 pos;
        vec2 vel;
        vec4 col;
    };
    layout(std430, binding = 0) buffer ParticleBuffer
    {
        Particle particles[];
    };

    uniform vec2 u_ray_point;
    uniform float u_damping;
    uniform float u_current;
    uniform float u_dt;
    uniform uint u_particle_count;
    uniform float u_particle_col;

    void main()
    {
            uint i = gl_GlobalInvocationID.x; // 현재 파티클의 인덱스 가져오기
            if (i >= u_particle_count) return; // 범위를 벗어나면 중단

            vec2 pos = particles[i].pos;
            vec2 vel = particles[i].vel;

            // cursor
            float r = length(u_cursor_pos - pos);
            r = max(r, 5.0);
            vec2 dir = (u_cursor_pos - pos) / r;
            vec2 tangent_dir = vec2(-dir.y, dir.x);


            // 1. 자기장의 방향 
            vec2 B_dir = vec2(-dir.y, dir.x);

            // 2. 자기장의 세기 (거리에 반비례)
            float B_mag = u_current / r;

            // 3. 입자의 움직임: 입자가 자기장 선을 따라 흐르게 만듦
            // 보정 ( 수치적 오차로 인한 원 커짐 방지 )
            vec2 final_B_dir = normalize(B_dir * 0.9 + dir * 0.1);
            vec2 target_vel = final_B_dir * B_mag * 100.0;

            vel = mix(vel, target_vel, 0.1);
            pos = pos + vel * u_dt;
            
            // color
            float speed = length(vel);
            float t = speed * 0.005;
            float intensity = clamp(t, 0.0, 1.0);
            particles[i].col = vec4(intensity, u_particle_col, 1.0 - intensity, 1.0);

            // save
            particles[i].pos = pos;
            particles[i].vel = vel;
    }
)";
const char *grid_vs_source = R"(
#version 430 core

layout(std430, binding = 2) buffer Grid
{
    vec4 grid_vertex[];
};
uniform mat4 view;
uniform mat4 projection;
out vec4 worldPos;

void main() {
    vec4 aPos = grid_vertex[gl_VertexID];
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