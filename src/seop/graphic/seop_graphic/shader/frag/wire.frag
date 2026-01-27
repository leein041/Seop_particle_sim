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