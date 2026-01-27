
#version 430 core
layout (location = 0) in vec4 i_pos;
layout (location = 1) in vec4 i_col;
layout (location = 2) in float i_size;
layout (location = 3) in float i_coord;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec4 o_col;
out float o_coord;

void main() {
    gl_PointSize = i_size;
    gl_Position =u_projection* u_view * i_pos;
    o_col = i_col;
    o_coord = i_coord;
}
