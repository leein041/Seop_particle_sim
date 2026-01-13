#pragma once

namespace seop::graphic
{
const char* gravity_cs_source =
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
}