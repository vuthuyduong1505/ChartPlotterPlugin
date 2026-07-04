#version 330 core

// Đầu ra là màu sắc của pixel
out vec4 FragColor;

// Nhận màu sắc từ lệnh setUniformValue trong C++
uniform vec4 ourColor;
uniform int u_lineStyle; // 0: Solid, 1: Dashed, 2: Dotted

void main() {
    if (u_lineStyle == 1) { // Dashed
        // Họa tiết gạch gạch dựa trên tọa độ pixel màn hình (mod 16: 8 pixel vẽ, 8 pixel nghỉ)
        if (int(gl_FragCoord.x + gl_FragCoord.y) % 16 < 8) {
            discard;
        }
    } else if (u_lineStyle == 2) { // Dotted
        // Họa tiết chấm chấm dựa trên tọa độ pixel màn hình (mod 6: 2 pixel vẽ, 4 pixel nghỉ)
        if (int(gl_FragCoord.x + gl_FragCoord.y) % 6 < 4) {
            discard;
        }
    }
    // Tô màu cho đồ thị theo màu nhận được từ C++
    FragColor = ourColor;
}