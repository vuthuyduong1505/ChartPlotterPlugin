#version 330 core

// Đầu ra là màu sắc của pixel
out vec4 FragColor;

// Nhận màu sắc từ lệnh setUniformValue trong C++
uniform vec4 ourColor;

void main() {
    // Tô màu cho đồ thị theo màu nhận được từ C++
    FragColor = ourColor;
}