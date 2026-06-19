#version 330 core

// Nhận vào tọa độ x, y, z từ C++ (location = 0 khớp với lệnh setAttributeBuffer)
layout (location = 0) in vec3 aPos;

//uniform float zoom;
//uniform float offsetX;

void main() {
    // tính toán tọa độ mới dựa trên zoom và offsetX
    //float newX=aPos.x*zoom+offsetX; // chỉ tác động vào trục X

    // Chuyển tọa độ vào biến hệ thống của OpenGL
    gl_Position = vec4(aPos.x,-aPos.y,aPos.z, 1.0);
}