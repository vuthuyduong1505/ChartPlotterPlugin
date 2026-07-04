#include "barchartstrategy.h"

BarChartStrategy::BarChartStrategy(): program(nullptr){}

BarChartStrategy::~BarChartStrategy()
{
    vboBar.destroy();
    vao.destroy();
    delete program;
}

void BarChartStrategy::init()
{
    program=new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader.frag");

    program->bind();
    vao.create();
    vao.bind();
    vboBar.create();
    vboBar.bind();
    program->setAttributeBuffer(0,GL_FLOAT,0,3,3*sizeof(float));
    program->enableAttributeArray(0);
    vao.release();

}
void BarChartStrategy::draw(QOpenGLFunctions *f, float time, const QColor &color,
                             const std::vector<DataPoint> &rawData,
                             float minX, float maxX, float minY, float maxY,
                             bool dataDirty)
{
    if (rawData.empty()) return;

    // Trục Y bảo vệ chia cho 0
    float renderMinY = minY;
    float renderMaxY = maxY;
    if (std::abs(renderMaxY - renderMinY) < 0.0001f) {
        renderMaxY += 1.0f;
        renderMinY -= 1.0f;
    }

    // Tỷ lệ khoảng đệm 20% cho trục Y
    float paddingY = (renderMaxY - renderMinY) * 0.2f;
    renderMinY -= paddingY;
    renderMaxY += paddingY;

    program->bind();
    vao.bind();

    // Thiết lập Uniforms cho Shader để mapping (Sử dụng camera động từ minX, maxX)
    program->setUniformValue("u_useMapping", 1);
    program->setUniformValue("u_minX", minX);
    program->setUniformValue("u_maxX", maxX);
    program->setUniformValue("u_minY", renderMinY);
    program->setUniformValue("u_maxY", renderMaxY);
    program->setUniformValue("u_mapMinX", -0.9f); // Chừa lề để cột không sát mép
    program->setUniformValue("u_mapMaxX", 0.9f);
    program->setUniformValue("u_mapMinY", -1.0f);
    program->setUniformValue("u_mapMaxY", 1.0f);

    QVector4D glColor(color.redF(), color.greenF(), color.blueF(), 1.0f);
    program->setUniformValue("ourColor", glColor);

    // TỐI ƯU: Chỉ tính toán đỉnh và nạp VBO khi dữ liệu thay đổi
    if (dataDirty || !vboBar.isCreated()) {
        barData.clear();
        barData.reserve(rawData.size() * 18); // 6 đỉnh * 3 tọa độ (x, y, z)

        // Tính toán chiều rộng cột Bar dựa trên độ rộng của dải dữ liệu X (Sử dụng camera động)
        float dataRangeX = maxX - minX;
        if (dataRangeX == 0.0f) dataRangeX = 1.0f;
        float barWidth = 0.015f * dataRangeX; // Chiều rộng bằng 1.5% dải dữ liệu
        float yBottom = renderMinY; // Đáy cột ở giá trị y nhỏ nhất

        for (const auto &p : rawData) {
            // Tam giác 1
            barData.push_back(p.x - barWidth); barData.push_back(yBottom); barData.push_back(0); // Trái dưới
            barData.push_back(p.x + barWidth); barData.push_back(yBottom); barData.push_back(0); // Phải dưới
            barData.push_back(p.x - barWidth); barData.push_back(p.y);     barData.push_back(0); // Trái trên

            // Tam giác 2
            barData.push_back(p.x - barWidth); barData.push_back(p.y);     barData.push_back(0); // Trái trên
            barData.push_back(p.x + barWidth); barData.push_back(yBottom); barData.push_back(0); // Phải dưới
            barData.push_back(p.x + barWidth); barData.push_back(p.y);     barData.push_back(0); // Phải trên
        }

        if (!vboBar.isCreated()) {
            vboBar.create();
        }
        vboBar.bind();
        vboBar.allocate(barData.data(), barData.size() * sizeof(float));
    } else {
        vboBar.bind();
    }

    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);

    f->glDrawArrays(GL_TRIANGLES, 0, barData.size() / 3);

    vao.release();
    program->release();
}

