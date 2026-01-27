#version 430 core
in vec4 vColor;
out vec4 FragColor;
void main() {
float dist = length(gl_PointCoord - vec2(0.5));
    
    // 중심은 밝고 외곽은 투명하게 (Glow 효과)
    float glow = exp(-dist * 1.0); // 숫자가 클수록 중심에 집중됨
    
    if (dist > 0.5) discard; // 원 밖은 버림

    FragColor = vec4(vColor.rgb, vColor.a * glow);
}