#version 430 core


layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;
uniform mat4 u_view;
uniform mat4 u_projection;
out vec4 worldPos;

void main() {
    worldPos = vec4(aPos.xyz * 100000.0, 1.0);
    gl_Position = u_projection * u_view * worldPos ;
}
