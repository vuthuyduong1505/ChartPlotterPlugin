#version 330 core

layout (location = 0) in vec3 aPos;

// Uniforms cho phép mapping tọa độ tuyến tính ngay trên GPU
uniform int u_useMapping; // 1: dùng mapping (cho Line, Bar), 0: không dùng (cho Grid, Axes, Pie)
uniform float u_minX;
uniform float u_maxX;
uniform float u_minY;
uniform float u_maxY;
uniform float u_mapMinX;
uniform float u_mapMaxX;
uniform float u_mapMinY;
uniform float u_mapMaxY;

void main() {
    if (u_useMapping == 1) {
        // Tránh chia cho 0 nếu minX == maxX hoặc minY == maxY
        float denX = (u_maxX - u_minX) == 0.0 ? 0.001 : (u_maxX - u_minX);
        float denY = (u_maxY - u_minY) == 0.0 ? 0.001 : (u_maxY - u_minY);

        float xGL = (u_mapMaxX - u_mapMinX) * (aPos.x - u_minX) / denX + u_mapMinX;
        float yGL = (u_mapMaxY - u_mapMinY) * (aPos.y - u_minY) / denY + u_mapMinY;

        // Chuyển tọa độ vào biến hệ thống của OpenGL (negate Y như thiết kế gốc)
        gl_Position = vec4(xGL, -yGL, aPos.z, 1.0);
    } else {
        gl_Position = vec4(aPos.x, -aPos.y, aPos.z, 1.0);
    }
}