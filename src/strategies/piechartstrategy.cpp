#include "piechartstrategy.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

pieChartStrategy::pieChartStrategy() : program(nullptr) {}

pieChartStrategy::~pieChartStrategy()
{
    for (auto &vbo : vboSlices) {
        vbo.destroy();
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
    vao.release();
    program->release();
}

std::vector<float> pieChartStrategy::computeSliceWeights(const std::vector<DataPoint> &rawData,
                                                         float minX, float maxX) const
{
    std::vector<float> weights;

    if (rawData.size() <= kMaxDirectSlices) {
        // Ít điểm: mỗi điểm là một lát, trọng số = giá trị Y (bỏ qua giá trị âm)
        weights.reserve(rawData.size());
        for (const auto &p : rawData) {
            weights.push_back(std::max(p.y, 0.0f));
        }
    } else {
        // Nhiều điểm (dạng time-series): gom theo khoảng X thành các nhóm
        weights.assign(kBinSliceCount, 0.0f);
        float rangeX = maxX - minX;
        if (rangeX < 1e-6f)
            rangeX = 1.0f;

        for (const auto &p : rawData) {
            int bin = static_cast<int>((p.x - minX) / rangeX * kBinSliceCount);
            bin = std::min(std::max(bin, 0), kBinSliceCount - 1);
            weights[bin] += std::max(p.y, 0.0f);
        }
    }

    // Loại bỏ lát có trọng số 0
    weights.erase(std::remove_if(weights.begin(), weights.end(),
                                 [](float w) { return w <= 0.0f; }),
                  weights.end());
    return weights;
}

void pieChartStrategy::rebuildSliceGeometry(QOpenGLFunctions *f, float scaleX, float scaleY)
{
    const int sliceCount = static_cast<int>(m_sliceWeights.size());
    if (sliceCount == 0) {
        m_vertexCounts.clear();
        return;
    }

    float total = 0.0f;
    for (float w : m_sliceWeights)
        total += w;
    if (total <= 0.0f)
        return;

    // Đảm bảo đủ VBO cho từng lát
    while (static_cast<int>(vboSlices.size()) < sliceCount) {
        QOpenGLBuffer vbo;
        vbo.create();
        vboSlices.push_back(std::move(vbo));
    }
    while (static_cast<int>(vboSlices.size()) > sliceCount) {
        vboSlices.back().destroy();
        vboSlices.pop_back();
    }

    m_vertexCounts.assign(sliceCount, 0);

    const float radius = 0.6f;
    const int segment = 30;
    float startAngle = 0.0f;

    for (int i = 0; i < sliceCount; ++i) {
        const float ratio = m_sliceWeights[i] / total;
        const float sliceAngle = ratio * 2.0f * static_cast<float>(M_PI);

        if (sliceAngle <= 0.0f)
            continue;

        std::vector<float> pieData;
        pieData.reserve(segment * 9);

        for (int j = 0; j < segment; ++j) {
            const float alpha1 = startAngle + (static_cast<float>(j) / segment) * sliceAngle;
            const float alpha2 = startAngle + (static_cast<float>(j + 1) / segment) * sliceAngle;

            pieData.push_back(0.0f);
            pieData.push_back(0.0f);
            pieData.push_back(0.0f);

            pieData.push_back(radius * std::cos(alpha1) * scaleX);
            pieData.push_back(radius * std::sin(alpha1) * scaleY);
            pieData.push_back(0.0f);

            pieData.push_back(radius * std::cos(alpha2) * scaleX);
            pieData.push_back(radius * std::sin(alpha2) * scaleY);
            pieData.push_back(0.0f);
        }

        vboSlices[i].bind();
        vboSlices[i].allocate(pieData.data(), static_cast<int>(pieData.size() * sizeof(float)));
        m_vertexCounts[i] = static_cast<int>(pieData.size() / 3);

        startAngle += sliceAngle;
    }
}

QVector4D pieChartStrategy::sliceColor(const QColor &base, int index, int total) const
{
    const float t = total > 1 ? static_cast<float>(index) / static_cast<float>(total - 1) : 0.0f;
    float factor = 0.65f + 0.35f * (1.0f - t);
    if (index % 2 == 1)
        factor *= 0.88f;

    return QVector4D(
        std::min(base.redF() * factor, 1.0f),
        std::min(base.greenF() * factor, 1.0f),
        std::min(base.blueF() * factor, 1.0f),
        1.0f);
}

void pieChartStrategy::draw(QOpenGLFunctions *f, float time, const QColor &color,
                            const std::vector<DataPoint> &rawData,
                            float minX, float maxX, float minY, float maxY,
                            bool dataDirty)
{
    Q_UNUSED(time);
    Q_UNUSED(minY);
    Q_UNUSED(maxY);

    if (rawData.empty())
        return;

    program->bind();
    vao.bind();
    program->setUniformValue("u_useMapping", 0);

    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);
    const int screenWidth = viewport[2];
    const int screenHeight = viewport[3];

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    if (screenWidth > screenHeight) {
        scaleX = static_cast<float>(screenHeight) / screenWidth;
    } else if (screenHeight > 0) {
        scaleY = static_cast<float>(screenWidth) / screenHeight;
    }

    const bool viewportChanged = screenWidth != m_lastViewportW || screenHeight != m_lastViewportH;
    if (dataDirty || viewportChanged || m_sliceWeights.empty()) {
        m_sliceWeights = computeSliceWeights(rawData, minX, maxX);
        rebuildSliceGeometry(f, scaleX, scaleY);
        m_lastViewportW = screenWidth;
        m_lastViewportH = screenHeight;
    }

    const int sliceCount = static_cast<int>(m_vertexCounts.size());
    for (int i = 0; i < sliceCount; ++i) {
        if (m_vertexCounts[i] == 0)
            continue;

        vboSlices[i].bind();
        program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
        program->enableAttributeArray(0);
        program->setUniformValue("ourColor", sliceColor(color, i, sliceCount));
        f->glDrawArrays(GL_TRIANGLES, 0, m_vertexCounts[i]);
    }

    vao.release();
    program->release();
}
