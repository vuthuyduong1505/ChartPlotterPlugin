#include "piechartstrategy.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

pieChartStrategy::pieChartStrategy() : program(nullptr) {}

pieChartStrategy::~pieChartStrategy()
{
    for (int i = 0; i < 3; ++i) {
        vboPie[i].destroy();
    }
    vao.destroy();
    delete program;
}

void pieChartStrategy::init()
{
    program = new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader.frag");
    program->link();

    vao.create();
    vao.bind();

    for (int i = 0; i < 3; ++i) {
        vboPie[i].create();
    }

    vao.release();
    program->release();
}

void pieChartStrategy::draw(QOpenGLFunctions *f, float time, const QColor &color,
                            const std::vector<DataPoint> &rawData,
                            float minX, float maxX, float minY, float maxY,
                            bool dataDirty)
{
    if (rawData.empty()) return;

    program->bind();
    vao.bind();

    // Bypass mapping vì tọa độ tròn đã được tính trực tiếp trong không gian GL
    program->setUniformValue("u_useMapping", 0);

    // Lấy kích thước cửa sổ và tính tỉ lệ aspect ratio
    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);
    int screenWidth = viewport[2];
    int screenHeight = viewport[3];

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    if (screenWidth > screenHeight) {
        scaleX = static_cast<float>(screenHeight) / screenWidth;
    } else {
        scaleY = static_cast<float>(screenWidth) / screenHeight;
    }

    // TỐI ƯU: Chỉ tính toán lát bánh và nạp VBO khi dữ liệu thay đổi
    if (dataDirty || m_vertexCount[0] == 0) {
        float countLow = 0, countMid = 0, countHigh = 0;
        for (const auto &p : rawData) {
            if (p.y <= 33) countLow++;
            else if (p.y <= 66) countMid++;
            else countHigh++;
        }

        float total = countHigh + countLow + countMid;
        if (total == 0.0f) total = 1.0f;
        float ratios[3] = { countLow / total, countMid / total, countHigh / total };

        float startAngle = 0.0f;
        float radius = 0.6f;

        for (size_t i = 0; i < 3; ++i) {
            if (ratios[i] == 0) {
                m_vertexCount[i] = 0;
                continue;
            }

            float sliceAngle = ratios[i] * 2.0f * M_PI;
            std::vector<float> pieData;
            int segment = 30;
            pieData.reserve(segment * 9); // 3 đỉnh * 3 tọa độ (x, y, z)

            for (int j = 0; j < segment; ++j) {
                float alpha1 = startAngle + (static_cast<float>(j) / segment) * sliceAngle;
                float alpha2 = startAngle + (static_cast<float>(j + 1) / segment) * sliceAngle;

                // Đỉnh 1: Tâm hình tròn
                pieData.push_back(0.0f); pieData.push_back(0.0f); pieData.push_back(0.0f);
                // Đỉnh 2: Điểm tại góc alpha1
                pieData.push_back(radius * std::cos(alpha1) * scaleX);
                pieData.push_back(radius * std::sin(alpha1) * scaleY);
                pieData.push_back(0.0f);
                // Đỉnh 3: Điểm tại góc alpha2
                pieData.push_back(radius * std::cos(alpha2) * scaleX);
                pieData.push_back(radius * std::sin(alpha2) * scaleY);
                pieData.push_back(0.0f);
            }

            vboPie[i].bind();
            vboPie[i].allocate(pieData.data(), pieData.size() * sizeof(float));
            m_vertexCount[i] = pieData.size() / 3;

            startAngle += sliceAngle;
        }
    }

    // Vẽ từ 3 VBO độc lập đã nạp sẵn bằng bộ Uniform Color
    std::vector<QVector4D> colors = {
        QVector4D(color.redF() * 0.7f, color.greenF() * 0.7f, color.blueF() * 0.7f, 1.0f), // Miếng 1: Tối hơn
        QVector4D(color.redF(),        color.greenF(),        color.blueF(),        1.0f), // Miếng 2: Màu gốc
        QVector4D(std::min(color.redF() * 1.3f, 1.0f), std::min(color.greenF() * 1.3f, 1.0f), std::min(color.blueF() * 1.3f, 1.0f), 1.0f) // Miếng 3: Sáng hơn
    };

    for (size_t i = 0; i < 3; ++i) {
        if (m_vertexCount[i] == 0) continue;

        vboPie[i].bind();
        program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
        program->enableAttributeArray(0);

        program->setUniformValue("ourColor", colors[i]);
        f->glDrawArrays(GL_TRIANGLES, 0, m_vertexCount[i]);
    }

    vao.release();
    program->release();
}
