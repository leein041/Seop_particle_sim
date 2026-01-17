#pragma once

namespace seop::graphic
{
const char* electromagnetic_cs_source = // B = I / 2πr
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

             // E. 두 전하 생성 ( ±1C 가정 )
            vec3 positve_charge_pos = vec3(1000.0, 0.0, 0.0);
            vec3 negtive_charge_pos = vec3(700.0, 0.0, 0.0);

            // E. E = 1/4πε * q/r² = 112,994,350,282 *  1/r² 
            float dis_from_pos = max(5.0, length(pos - positve_charge_pos));
            float dis_from_neg = max(5.0, length(pos - negtive_charge_pos));
            vec3 dir_from_pos = normalize(pos - positve_charge_pos);
            vec3 dir_to_neg = normalize(negtive_charge_pos - pos);

            vec3 E = vec3(0.0);
            E += dir_from_pos * 112994350282.0 / (dis_from_pos * dis_from_pos);
            E += dir_to_neg * 112994350282.0 / (dis_from_neg * dis_from_neg);

            // B. 전류 가정 ( +y 방향 )
            // B. B = μ₀/4π ∫I*r̂/r² dl ( bio-savar )
            // 무한히 긴 도선인 경우 B = ̂φ * μ₀I/2πr (μ₀ = 4π x 10e-7) = 12.5663 x 10e-7
            float B_scale = 1000.0;
            vec3 B = vec3(0.0);
            vec3 I_dir = vec3(0.0,1.0,0.0);
            vec3 r = vec3(pos.x,0.0,pos.z);
            float r_dis = max(5.0, length(r));
            vec3 r_dir = normalize(r);
            vec3 B_dir = cross(I_dir,r_dir);
            B += B_dir * 4 * B_scale / (2 * r_dis);
            

            if(i % 2 == 0){
                particles[i].col = vec4(1.0, 0.0, 0.0, 1.0);
               vel = E;

                // 4. 입자 재생성 (음전하에 너무 가까워지면 양전하로 점프)
                float reset_dist = 10.0; // 음전하와 입자 사이의 거리 임계값

                if (dis_from_neg < reset_dist) {
                uint r1_uint = pcg_hash(i);
                uint r2_uint = pcg_hash(i + u_particle_count); // 오프셋으로 겹침 방지
                uint r3_uint = pcg_hash(i + u_particle_count * 2u);

                float r1 = uint_to_float(r1_uint);
                float r2 = uint_to_float(r2_uint);
                float r3 = uint_to_float(r3_uint);

                float pi =  3.141592;

                vec3 sphere_offset;
                // XXX : why?
                float cos_theta = r1 * 2.0 - 1.0;          // -1 ~ 1 균등하게
                float sin_theta = sqrt(max(0.0, 1.0 - cos_theta * cos_theta));
                // 
                // this is my code
                // float cos_theta = cos(r1 * pi);
                // float sin_theta = sin(r1 * pi);

                float cos_phi = cos(r2 * 2.0 * pi); 
                float sin_phi = sin(r2 * 2.0 * pi); 
                float r = r3 * 100.0;

                sphere_offset.x = r * sin_theta * cos_phi;
                sphere_offset.z = r * sin_theta * sin_phi;
                sphere_offset.y = r * cos_theta; 

                pos = positve_charge_pos + sphere_offset;
                vel = normalize(sphere_offset) * 1.0;
                }
            }else{
                vel = B;       

                particles[i].col = vec4(0.0, 1.0, 0.0, 1.0);
            }
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