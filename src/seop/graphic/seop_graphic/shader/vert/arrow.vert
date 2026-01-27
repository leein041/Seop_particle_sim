#version 430 core

layout (location = 0) in vec4 i_pos;
layout (location = 1) in vec4 i_field;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform bool u_view_electric_field;
uniform bool u_view_magnetic_field;
uniform bool u_view_poynting_field;
uniform float u_arrow_scale;

out vec4 o_color;

void main() {
    vec3 final_pos = i_pos.xyz;
    vec4 final_col = vec4(1,1,1,1);

    if (gl_VertexID % 6 == 1 && u_view_electric_field) { // 전기장
        final_pos += i_field.xyz * i_field.w * u_arrow_scale;
        final_col = vec4(1,0.2,0.2,1.0);
    } else if( gl_VertexID % 6 == 3 && u_view_magnetic_field){ // 자기장
        final_pos += i_field.xyz * i_field.w * u_arrow_scale;
        final_col = vec4(0.2,0.2,1,1.0);
    }  else if( gl_VertexID % 6 == 5 && u_view_poynting_field){ // 포인팅 벡터
        final_pos += i_field.xyz * i_field.w * u_arrow_scale;
        final_col = vec4(1.0,0.4,1,1.0);
    } 
    gl_Position = u_projection * u_view * vec4(final_pos.xyz, 1.0);
    o_color = final_col;
}