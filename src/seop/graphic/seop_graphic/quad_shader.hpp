#pragma once
namespace seop::graphic
{
const char *quad_vs_source = R"(
#version 430 core
layout (location = 0) in vec4 i_pos;
layout (location = 1) in vec4 i_uv;
out vec4 o_uv;

void main() {
    o_uv = i_uv; 
    gl_Position = i_pos;
}
)";

const char *quad_fs_source = R"(    
#version 430 core
in vec4 o_uv;
out vec4 o_color;

uniform sampler2D u_screen_texture;
uniform vec4 u_color;
uniform bool use_texture;

void main() {

    if(use_texture){
    o_color = texture(u_screen_texture, o_uv.xy);
    } else {
    o_color = u_color;
    }
}

)";

} // namespace