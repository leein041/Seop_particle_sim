
#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;
out vec4 vColor;

uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    gl_Position =u_projection* u_view * aPos;
                        
    vColor = aColor; 
}

