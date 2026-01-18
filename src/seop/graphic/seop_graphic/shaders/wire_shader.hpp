#pragma once

namespace seop::graphic
{
const char* wire_vs_source = R"(
#version 430 core
layout (location = 0) in vec4 aPos;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position =projection* view * aPos;
}
)";

const char* wire_fs_source = R"(    
#version 430 core
out vec4 FragColor;
uniform vec4 u_color;

void main() {
    FragColor = u_color;
}
)";
} // namespace seop::graphic