#pragma once

namespace seop::graphic
{
const char* Magnetic_field_by_square_conductor_cs_source = // B = I / 2πr
    R"(#version 430 core
        layout(local_size_x = 256) in; // 워크그룹 크기

    struct Particle
    {
        vec4 pos;
        vec4 col;
        vec4 vel;
    };
    layout(std430, binding = 0) buffer ParticleBuffer
    {
        Particle particles[];
    };

    uniform float u_dt;
    uniform float u_time_scale;
    uniform uint u_particle_count;
    uniform float u_particle_col;
    uniform float u_damping;

    uint pcg_hash(uint v) {
        uint state = v * 747796405u + 2891336453u;
        uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
    }

    // 0.0 ~ 1.0 사이의 float로 변환
    float uint_to_float(uint v) {
        return float(v) * (1.0 / 4294967296.0);
    }

    void main()
    {
            uint i = gl_GlobalInvocationID.x; // 현재 파티클의 인덱스 가져오기
            if (i >= u_particle_count) return; // 범위를 벗어나면 중단

            vec3 pos = particles[i].pos.xyz;
            vec3 vel = particles[i].vel.xyz;

            // B. 전류 가정 ( +y 방향 )
            // B. B = μ₀/4π ∫I*r̂/r² dl ( bio-savar )
            // 무한히 긴 도선인 경우 B = ̂φ * μ₀I/2πr (μ₀ = 4π x 10e-7) = 12.5663 x 10e-7
            vec3 B = vec3(0.0);
            vec3 I_dir = vec3(0.0,1.0,0.0);
            vec3 r = vec3(pos.x,0.0,pos.z);
            float r_dis = max(5.0, length(r));
            vec3 r_dir = normalize(r);
            vec3 B_dir = cross(I_dir,r_dir);
            B += B_dir * 4 * 1 / (2 * r_dis);
            // 시각적 표현 위한 스케일링
            B *= 10.0;
            vel = B;       

            particles[i].col = vec4(0.0, 1.0, 0.0, 1.0);
            
            if(length(vel) > 300.0)
            {
                vel = normalize(vel) * 300;
            }
            pos = pos + vel  * u_dt * u_time_scale;

            // save
            particles[i].pos.xyz = pos;
            particles[i].vel.xyz = vel;

            // color
            //float speed = length(vel);
            //float t = speed * 0.001;
            //float intensity = clamp(t, 0.0, 1.0);
            //particles[i].col = vec4(intensity, u_particle_col, 1.0 - intensity, 1.0);
            
    }
)";
}