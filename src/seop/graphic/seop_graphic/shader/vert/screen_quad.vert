#version 430 core
layout (location = 0) in vec4 i_pos;
layout (location = 1) in vec4 i_uv;
out vec4 o_uv;

void main() {
    o_uv = i_uv; 
    gl_Position = i_pos;
}
