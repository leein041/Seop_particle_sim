#pragma once

namespace seop::graphic
{
const char* magnetic_field_by_square_cs_source = R"(#version 430 core
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
uniform float u_time_scale;
uniform float u_particle_col; 

void main()
{
    uint i = gl_GlobalInvocationID.x; // 현재 파티클의 인덱스 가져오기
    if (i >= u_particle_count) return; // 범위를 벗어나면 중단
    vec3 pos = particles[i].pos.xyz;
    vec3 vel = particles[i].vel.xyz;

    // B. 전류 가정 ( 정사각형 도선 )
    // B. B = μ₀/4π ∫l̂*r̂/r² dl ( bio-savar )

    vec3 B = vec3(0.0);
    for(int j = 0; j<u_wire_count; j++){
        float I = 10000000.0; 
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

        vec3 B_dir = normalize(cross(L, HP));
        float r = length(HP);
        if (r < 2.0) continue;   
        B += B_dir * I / (4.0 * 3.1415 * r) * (sina - sinb);
    }

    vel = B;       

    if(length(vel) > 300.0)
    {
        vel = normalize(vel) * 300;
    }
    pos = pos + vel  * u_dt * u_time_scale;

    // save
    particles[i].pos.xyz = pos;
    particles[i].vel.xyz = vel;

    // color
    float speed = length(vel);
    float col_t = speed * 0.001;
    float intensity = clamp(col_t, 0.0, 1.0);
    particles[i].col = vec4(intensity, u_particle_col, 1.0 - intensity, 1.0);
    
}
)";
}