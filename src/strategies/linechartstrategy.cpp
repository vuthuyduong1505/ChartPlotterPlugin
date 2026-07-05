#include "linechartstrategy.h"
#include <cmath>

LineChartStrategy::LineChartStrategy() :program(nullptr){}

LineChartStrategy::~LineChartStrategy()
{
    vboLine.destroy();
    vboAxes.destroy();
    vboGrid.destroy();
    vao.destroy();
    delete program;
}

void LineChartStrategy::init()
{
    program = new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex,  ":/shaders/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader.frag");
    program->bind();

    vao.create();
    vao.bind();

    vboGrid.create();
    vboAxes.create();
    vboLine.create();

    vao.release();
    program->release();
}

static float calculateGridStep(float range)
{
    float rawStep = range / 8.0f;
    if (rawStep <= 0.0f) return 1.0f;
    float logStep = std::log10(rawStep);
    float exponent = std::floor(logStep);
    float base = std::pow(10.0f, exponent);
    float fraction = rawStep / base;

    float step;
    if (fraction < 1.5f) step = 1.0f * base;
    else if (fraction < 3.0f) step = 2.0f * base;
    else if (fraction < 7.0f) step = 5.0f * base;
    else step = 10.0f * base;
    return step;
}

void LineChartStrategy::draw(QOpenGLFunctions *f, float time, const QColor &color,
                             const std::vector<DataPoint> &rawData,
                             float minX, float maxX, float minY, float maxY,
                             bool dataDirty, int lineStyle)
{
    Q_UNUSED(time);
    if (rawData.empty()) return;

    // Trục Y bảo vệ chia cho 0
    float renderMinY = minY;
    float renderMaxY = maxY;
    if (std::abs(renderMaxY - renderMinY) < 0.0001f) {
        renderMaxY += 1.0f;
        renderMinY -= 1.0f;
    }

    program->bind();
    vao.bind();

    // 1. Tính bước lưới
    float stepX = calculateGridStep(maxX - minX);
    float stepY = calculateGridStep(renderMaxY - renderMinY);
    float denX = (maxX - minX) == 0.0f ? 0.001f : (maxX - minX);
    float denY = (renderMaxY - renderMinY) == 0.0f ? 0.001f : (renderMaxY - renderMinY);

    gridData.clear();

    // Đường lưới dọc
    float firstGridX = std::ceil(minX / stepX) * stepX;
    for (float valX = firstGridX; valX <= maxX; valX += stepX) {
        if (valX < minX - 1e-5f) continue;
        float xGL = 2.0f * (valX - minX) / denX - 1.0f;
        if (xGL > 1.05f) break;
        gridData.push_back(xGL); gridData.push_back(-1.0f); gridData.push_back(0.0f);
        gridData.push_back(xGL); gridData.push_back(1.0f);  gridData.push_back(0.0f);
    }

    // Đường lưới ngang
    float firstGridY = std::ceil(renderMinY / stepY) * stepY;
    for (float valY = firstGridY; valY <= renderMaxY; valY += stepY) {
        if (valY < renderMinY - 1e-5f) continue;
        float yGL = 2.0f * (valY - renderMinY) / denY - 1.0f;
        float yGL_neg = -yGL;
        if (yGL > 1.05f) break;
        gridData.push_back(-1.0f); gridData.push_back(yGL_neg); gridData.push_back(0.0f);
        gridData.push_back(1.0f);  gridData.push_back(yGL_neg); gridData.push_back(0.0f);
    }

    // 2. Vẽ lưới (bypass mapping) - Màu xám mờ giống Bar Chart
    program->setUniformValue("u_useMapping", 0);
    program->setUniformValue("u_lineStyle", 0);
    program->setUniformValue("ourColor", QVector4D(0.55f, 0.60f, 0.65f, 0.30f));
    vboGrid.bind();
    vboGrid.allocate(gridData.data(), gridData.size() * sizeof(float));
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    f->glLineWidth(1.0f);
    f->glDrawArrays(GL_LINES, 0, gridData.size() / 3);
    f->glDisable(GL_BLEND);

    // 3. Vẽ trục biên L-shape
    program->setUniformValue("ourColor", QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    float borderAxesData[] = {
        -1.0f, -1.0f, 0.0f,   1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,  -1.0f,  1.0f, 0.0f
    };
    vboAxes.bind();
    vboAxes.allocate(borderAxesData, sizeof(borderAxesData));
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);
    f->glLineWidth(2.0f);
    f->glDrawArrays(GL_LINES, 0, 4);

    // 4. Vẽ đồ thị Line với mapping động
    program->setUniformValue("u_useMapping", 1);
    program->setUniformValue("u_lineStyle", lineStyle);
    program->setUniformValue("u_minX", minX);
    program->setUniformValue("u_maxX", maxX);
    program->setUniformValue("u_minY", renderMinY);
    program->setUniformValue("u_maxY", renderMaxY);
    program->setUniformValue("u_mapMinX", -1.0f);
    program->setUniformValue("u_mapMaxX", 1.0f);
    program->setUniformValue("u_mapMinY", -1.0f);
    program->setUniformValue("u_mapMaxY", 1.0f);

    QVector4D glColor(color.redF(), color.greenF(), color.blueF(), 1.0f);
    program->setUniformValue("ourColor", glColor);

    // Chỉ upload VBO khi dữ liệu thay đổi
    if (dataDirty || !vboLine.isCreated()) {
        plotData.clear();
        plotData.reserve(rawData.size() * 3);
        for (const auto& p : rawData) {
            plotData.push_back(p.x);
            plotData.push_back(p.y);
            plotData.push_back(0.0f);
        }
        if (!vboLine.isCreated()) vboLine.create();
        vboLine.bind();
        vboLine.allocate(plotData.data(), plotData.size() * sizeof(float));
    } else {
        vboLine.bind();
    }

    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);

    f->glLineWidth(2.0f);
    f->glDrawArrays(GL_LINE_STRIP, 0, plotData.size() / 3);

    vao.release();
    program->release();
}
