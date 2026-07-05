#include "barchartstrategy.h"
#include <cmath>

static float barGridStep(float range)
{
    float rawStep = range / 8.0f;
    if (rawStep <= 0.0f) return 1.0f;
    float exponent = std::floor(std::log10(rawStep));
    float base = std::pow(10.0f, exponent);
    float fraction = rawStep / base;
    if (fraction < 1.5f) return 1.0f * base;
    if (fraction < 3.0f) return 2.0f * base;
    if (fraction < 7.0f) return 5.0f * base;
    return 10.0f * base;
}

BarChartStrategy::BarChartStrategy(): program(nullptr){}

BarChartStrategy::~BarChartStrategy()
{
    vboBar.destroy();
    vboGrid.destroy();
    vboAxes.destroy();
    vao.destroy();
    delete program;
}

void BarChartStrategy::init()
{
    program = new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader.frag");
    program->link();

    program->bind();
    vao.create();
    vao.bind();
    vboBar.create();
    vboGrid.create();
    vboAxes.create();
    vao.release();
    program->release();
}

void BarChartStrategy::draw(QOpenGLFunctions *f, float time, const QColor &color,
                             const std::vector<DataPoint> &rawData,
                             float minX, float maxX, float minY, float maxY,
                             bool dataDirty, int lineStyle)
{
    Q_UNUSED(time);
    Q_UNUSED(lineStyle);
    if (rawData.empty()) return;

    float renderMinY = minY;
    float renderMaxY = maxY;
    if (std::abs(renderMaxY - renderMinY) < 0.0001f) {
        renderMaxY += 1.0f;
        renderMinY -= 1.0f;
    }

    program->bind();
    vao.bind();

    // ─── Bước 1: Vẽ lưới ngang mờ (chỉ horizontal, không có dọc) ───
    float stepY = barGridStep(renderMaxY - renderMinY);
    float denY  = (renderMaxY - renderMinY) == 0.0f ? 0.001f : (renderMaxY - renderMinY);

    gridData.clear();
    float firstGridY = std::ceil(renderMinY / stepY) * stepY;
    for (float valY = firstGridY; valY <= renderMaxY + 1e-5f; valY += stepY) {
        float yGL = 2.0f * (valY - renderMinY) / denY - 1.0f;
        float yNeg = -yGL;  // shader dùng -aPos.y
        if (yGL >  1.05f) break;
        if (yGL < -1.05f) continue;
        gridData.push_back(-1.0f); gridData.push_back(yNeg); gridData.push_back(0.0f);
        gridData.push_back( 1.0f); gridData.push_back(yNeg); gridData.push_back(0.0f);
    }

    program->setUniformValue("u_useMapping", 0);
    program->setUniformValue("u_lineStyle",  0);
    // Màu xám mờ, alpha thấp để làm nổi bật cột
    program->setUniformValue("ourColor", QVector4D(0.55f, 0.60f, 0.65f, 0.30f));

    vboGrid.bind();
    vboGrid.allocate(gridData.data(), gridData.size() * sizeof(float));
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);

    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    f->glLineWidth(1.0f);
    f->glDrawArrays(GL_LINES, 0, gridData.size() / 3);

    // ─── Bước 2: Vẽ trục biên L-shape (trái + dưới) ───
    program->setUniformValue("ourColor", QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    float borderData[] = {
        -1.0f, -1.0f, 0.0f,   1.0f, -1.0f, 0.0f,   // trục ngang (dưới)
        -1.0f, -1.0f, 0.0f,  -1.0f,  1.0f, 0.0f    // trục dọc (trái)
    };
    vboAxes.bind();
    vboAxes.allocate(borderData, sizeof(borderData));
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);
    f->glLineWidth(2.0f);
    f->glDrawArrays(GL_LINES, 0, 4);

    // ─── Bước 3: Vẽ các cột với mapping động ───
    program->setUniformValue("u_useMapping", 1);
    program->setUniformValue("u_minX", minX);
    program->setUniformValue("u_maxX", maxX);
    program->setUniformValue("u_minY", renderMinY);
    program->setUniformValue("u_maxY", renderMaxY);
    program->setUniformValue("u_mapMinX", -1.0f);
    program->setUniformValue("u_mapMaxX",  1.0f);
    program->setUniformValue("u_mapMinY", -1.0f);
    program->setUniformValue("u_mapMaxY",  1.0f);
    program->setUniformValue("u_lineStyle", 0);

    QVector4D glColor(color.redF(), color.greenF(), color.blueF(), 1.0f);
    program->setUniformValue("ourColor", glColor);

    if (dataDirty || !vboBar.isCreated()) {
        barData.clear();
        barData.reserve(rawData.size() * 18);

        float dataRangeX = maxX - minX;
        if (dataRangeX == 0.0f) dataRangeX = 1.0f;
        float barWidth = 0.015f * dataRangeX;
        float yBottom  = renderMinY;

        for (const auto &p : rawData) {
            barData.push_back(p.x - barWidth); barData.push_back(yBottom); barData.push_back(0.0f);
            barData.push_back(p.x + barWidth); barData.push_back(yBottom); barData.push_back(0.0f);
            barData.push_back(p.x - barWidth); barData.push_back(p.y);     barData.push_back(0.0f);

            barData.push_back(p.x - barWidth); barData.push_back(p.y);     barData.push_back(0.0f);
            barData.push_back(p.x + barWidth); barData.push_back(yBottom); barData.push_back(0.0f);
            barData.push_back(p.x + barWidth); barData.push_back(p.y);     barData.push_back(0.0f);
        }

        if (!vboBar.isCreated()) vboBar.create();
        vboBar.bind();
        vboBar.allocate(barData.data(), barData.size() * sizeof(float));
    } else {
        vboBar.bind();
    }

    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);
    f->glDrawArrays(GL_TRIANGLES, 0, barData.size() / 3);

    f->glDisable(GL_BLEND);
    vao.release();
    program->release();
}
