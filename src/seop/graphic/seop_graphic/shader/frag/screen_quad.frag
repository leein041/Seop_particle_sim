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