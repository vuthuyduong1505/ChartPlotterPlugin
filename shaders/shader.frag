#version 330 core

in vec2 vPos;

// Đầu ra là màu sắc của pixel
out vec4 FragColor;

// Nhận màu sắc từ lệnh setUniformValue trong C++
uniform vec4 ourColor;
uniform int u_lineStyle; // 0: Solid, 1: Dashed, 2: Dotted
uniform int u_chartType; // 0: Line, 1: Bar, 2: Pie/Donut SDF

// Uniforms cho Donut SDF
uniform vec2 u_scale;
uniform int u_sliceCount;
uniform float u_sliceStartAngles[12];
uniform float u_sliceAngles[12];
uniform vec4 u_sliceColors[12];
uniform int u_hoveredSlice;
uniform float u_innerRadius;
uniform float u_outerRadius;
uniform float u_cornerRadius;
uniform float u_gap;
uniform float u_explodeOffset;

void main() {
    if (u_chartType == 2) {
        vec2 p = vec2(vPos.x / u_scale.x, vPos.y / u_scale.y);
        float minDist = 1000.0;
        vec4 bestColor = vec4(0.0);
        
        float rMid = 0.5 * (u_innerRadius + u_outerRadius);
        float tHalf = 0.5 * (u_outerRadius - u_innerRadius) - u_cornerRadius;

        for (int i = 0; i < u_sliceCount && i < 12; ++i) {
            float midAngle = u_sliceStartAngles[i] + 0.5 * u_sliceAngles[i];
            vec2 offset = vec2(0.0);
            if (i == u_hoveredSlice) {
                offset = vec2(cos(midAngle), sin(midAngle)) * u_explodeOffset;
            }
            vec2 q = p - offset;
            
            // Xoay q để lát bánh đối xứng qua trục X (gốc 0)
            float c = cos(-midAngle);
            float s = sin(-midAngle);
            vec2 w = vec2(c * q.x - s * q.y, s * q.x + c * q.y);
            w.y = abs(w.y);

            float alpha = max(0.0, 0.5 * (u_sliceAngles[i] - u_gap));
            vec2 nAlpha = vec2(-sin(alpha), cos(alpha));
            vec2 dir = vec2(cos(alpha), sin(alpha));

            float dr = abs(length(w) - rMid) - tHalf;
            
            // Khoảng cách góc dPhi chính xác: Loại bỏ hoàn toàn vạch ma ở phía đối diện
            float dPhi;
            if (dot(w, dir) <= 0.0) {
                dPhi = length(w); // Nếu ở phía sau hoặc đối diện hướng tia, khoảng cách lấy từ tâm
            } else {
                dPhi = dot(w, nAlpha);
            }

            vec2 d = vec2(dr, dPhi);
            float dist = length(max(d, 0.0)) + min(max(dr, dPhi), 0.0) - u_cornerRadius;

            if (dist < minDist) {
                minDist = dist;
                bestColor = u_sliceColors[i];
            }
        }

        float edgeWidth = max(fwidth(minDist), 0.002);
        float alpha = 1.0 - smoothstep(-edgeWidth, edgeWidth, minDist);
        if (alpha <= 0.001) discard;
        FragColor = vec4(bestColor.rgb, bestColor.a * alpha);
        return;
    }

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