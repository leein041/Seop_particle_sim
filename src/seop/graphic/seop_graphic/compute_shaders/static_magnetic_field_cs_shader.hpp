#pragma once

namespace seop::graphic
{
const char* static_magnetic_field_cs_source = R"(#version 430 core
layout(local_size_x = 256) in; // 워크그룹 크기
struct Particle
{
    vec4 pos;
    vec4 col;
    vec4 vel;
};
struct Conductor_vertex
{
    vec4 pos;
    vec4 col;
    float size;
    float pad[3];
};
layout(std430, binding = 0) buffer ParticleBuffer
{
    Particle particles[];
};
layout(std430, binding = 1) buffer ConductorBuffer // 새로운 바인딩 1번
{
    Conductor_vertex conductor_vertices[]; // 점의 개수가 유동적일 수 있으므로 배열로 선언
};

uniform uint u_wire_count;
uniform uint u_particle_count;

uniform float u_dt;
uniform float u_time;
uniform float u_time_scale;
uniform float u_particle_col; 
uniform float u_w;
uniform float u_max_i;

void main()
{
    uint i = gl_GlobalInvocationID.x; // 현재 파티클의 인덱스 가져오기
    if (i >= u_particle_count) return; // 범위를 벗어나면 중단
    vec3 pos = particles[i].pos.xyz;
    vec3 vel = particles[i].vel.xyz;
    float scale = 1000.0; // 입자 움직임 관찰하기 쉽게
    


    // B. B = μ₀/4π ∫l̂*r̂/r² dl ( bio-savar ) 유도장과 방사장은 다름. 방사장은 r^2 이 아닌 r 에 반비례..

    vec3 B = vec3(0.0);
    vec3 E = vec3(0.0);
    vec3 B_dir = vec3(0.0);
    vec3 E_dir =  vec3(0.0);
    vec3 total_force = vec3(0.0f);

    for(int j = 0; j<u_wire_count; j++){
        vec3 A = conductor_vertices[j * 2].pos.xyz;
        vec3 B_p = conductor_vertices[j * 2 + 1].pos.xyz;
        vec3 L = B_p - A;
        if (length(L) < 2.0) continue;   
        
        vec3 AP = pos - A;
        vec3 BP = pos - B_p;
        float t = dot(AP, L) / dot(L, L);
        vec3 H = A + t * L; // H = A + tv
        vec3 HP = pos - H;
        float sina = dot(normalize(L),normalize(AP));
        float sinb = dot(normalize(L),normalize(BP));

        float r = length(HP);
        if (r < 2.0) continue;   
        float c = 300.0; // 시뮬레이션 상의 빛의 속도 (적절히 조절)
        float delay = r / c;
        float retarded_time = u_time - delay;
    
        // 현재 u_current 대신, 거리만큼 지연된 시간의 전류값을 계산
        float I_delayed = u_max_i * sin(u_w * retarded_time);
        float dI_dt_delayed = u_max_i * u_w * cos(u_w * retarded_time);

        // 자기장
        B_dir = normalize(cross(L, HP));
        B += B_dir * I_delayed * scale / (4.0 * 3.1415 * r) * (sina - sinb);

        // 전기장
        E_dir =  normalize(L);
        E += -E_dir * dI_dt_delayed * scale / (4.0 * 3.1415 * r) * (sina - sinb);

        // 로런츠 힘, 입자가 받는 힘 F = q(E + u X B)
        total_force += 1.0 * (E + cross(vel, B));

    }
    vel += total_force * u_dt;
    vel *= 0.98;

    if(length(vel) > 300.0)
    {
        vel = normalize(vel) * 300;
    }
    pos = pos + vel  * u_dt * u_time_scale;

    // save
    particles[i].pos.xyz = pos;
    particles[i].vel.xyz = vel;

    float phase_col = dot(normalize(vel), E_dir) * 0.5 + 0.5;
    particles[i].col = vec4(phase_col, u_particle_col, 1.0 - phase_col, 1.0);
}
)";
}