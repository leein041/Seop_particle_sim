#pragma once

namespace seop::graphic
{
const char* wire_vs_source = R"(
#version 430 core
layout (location = 0) in vec4 i_pos;
layout (location = 1) in vec4 i_col;
layout (location = 2) in float i_size;
layout (location = 3) in float i_coord;

uniform mat4 view;
uniform mat4 projection;

out vec4 o_col;
out float o_coord;

void main() {
    gl_PointSize = i_size;
    gl_Position =projection* view * i_pos;
    o_col = i_col;
    o_coord = i_coord;
}
)";

const char* wire_fs_source = R"(    
#version 430 core
in vec4 o_col;
in float o_coord;
out vec4 FragColor;

uniform float u_time;
void main() {

float speed = u_time * 2.0;
float pattern = sin(o_coord * 50.0 - u_time * 5.0);

if (pattern < 0.0) {
    discard; // 또는 투명도 조절: FragColor = vec4(o_col.rgb, 0.2);
} else {
    FragColor = o_col;
}

}
)";
} // namespace seop::graphic