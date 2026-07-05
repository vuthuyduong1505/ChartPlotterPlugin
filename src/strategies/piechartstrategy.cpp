#include "piechartstrategy.h"
#include <cmath>
#include <algorithm>
#include <QVector2D>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

pieChartStrategy::pieChartStrategy() : program(nullptr) {}

pieChartStrategy::~pieChartStrategy()
{
    vboQuad.destroy();
    vao.destroy();
    delete program;
}

void pieChartStrategy::init()
{
    program = new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader.frag");
    program->link();

    float quadVertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f
    };

    vboQuad.create();
    vboQuad.bind();
    vboQuad.allocate(quadVertices, sizeof(quadVertices));

    vao.create();
    vao.bind();
    vboQuad.bind();
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);
    vao.release();
    program->release();
}

std::vector<PieSliceInfo> pieChartStrategy::computeSlices(const std::vector<DataPoint> &rawData,
                                                          float minX, float maxX, float minY, float maxY,
                                                          int binMode, const QColor &baseColor)
{
    Q_UNUSED(binMode);
    Q_UNUSED(minX);
    Q_UNUSED(maxX);
    Q_UNUSED(minY);
    Q_UNUSED(maxY);
    std::vector<PieSliceInfo> slices;
    if (rawData.empty()) return slices;

    size_t n = rawData.size();
    float actualMinY = rawData[0].y;
    float actualMaxY = rawData[0].y;
    double sumY = 0.0;
    double sumSqY = 0.0;

    // --- VÒNG LẶP 1: Tìm minY, maxY thực tế của file và thống kê Mean/Variance ---
    for (const auto &p : rawData) {
        float y = p.y;
        if (y < actualMinY) actualMinY = y;
        if (y > actualMaxY) actualMaxY = y;
        sumY += y;
        sumSqY += static_cast<double>(y) * y;
    }

    float rangeY = actualMaxY - actualMinY;
    
    // Xử lý hiển thị: Nếu dữ liệu thực sự giống hệt nhau (min == max), hiển thị 1 lát bánh duy nhất
    if (std::abs(rangeY) < 1e-6f) {
        PieSliceInfo s;
        s.weight = static_cast<float>(n);
        s.name = QString("Đồng nhất (%1)").arg(actualMinY, 0, 'f', 2);
        s.binMin = actualMinY;
        s.binMax = actualMaxY;
        slices.push_back(s);
    } else {
        double mean = sumY / n;
        double variance = (sumSqY / n) - (mean * mean);
        if (variance < 0.0) variance = 0.0;
        float sigma = static_cast<float>(std::sqrt(variance));

        // --- XỬ LÝ OUTLIERS (ĐIỂM DỊ BIỆT / SPIKES) ---
        // Nếu khoảng cách từ min đến max quá lớn so với độ lệch chuẩn (rangeY > 5 * sigma)
        bool hasOutliers = (sigma > 1e-6f) && (rangeY > 5.0f * sigma);
        
        float effMin = actualMinY;
        float effMax = actualMaxY;
        if (hasOutliers) {
            // Tách các điểm dị biệt bằng cách giới hạn vùng chính trong [mean - 2.5*sigma, mean + 2.5*sigma]
            effMin = std::max(actualMinY, static_cast<float>(mean - 2.5 * sigma));
            effMax = std::min(actualMaxY, static_cast<float>(mean + 2.5 * sigma));
            if (effMax - effMin < 1e-6f) {
                effMin = actualMinY;
                effMax = actualMaxY;
                hasOutliers = false;
            }
        }

        float effRange = effMax - effMin;
        if (effRange < 1e-6f) effRange = 1.0f;

        const QString levelNames[5] = {
            "Rất thấp",
            "Thấp",
            "Trung bình",
            "Cao",
            "Rất cao"
        };
        std::vector<float> mainCounts(5, 0.0f);
        float outlierCount = 0.0f;

        // --- VÒNG LẶP 2: Phân loại điểm vào 5 nhóm chính và gom tất cả Outliers vào 1 nhóm ---
        for (const auto &p : rawData) {
            float y = p.y;
            if (hasOutliers && (y < effMin || y > effMax)) {
                outlierCount += 1.0f; // Gom cụm Outliers (Spikes) vào duy nhất 1 nhóm
            } else {
                int idx = static_cast<int>((y - effMin) / effRange * 5.0f);
                if (idx < 0) idx = 0;
                if (idx > 4) idx = 4;
                mainCounts[idx] += 1.0f;
            }
        }

        // Xóa bỏ các lát bánh "ma": Chỉ thêm các nhóm thực sự có dữ liệu (> 0 điểm)
        for (int i = 0; i < 5; ++i) {
            if (mainCounts[i] > 0.0f) {
                PieSliceInfo s;
                s.weight = mainCounts[i];
                s.name = levelNames[i];
                s.binMin = effMin + i * (effRange / 5.0f);
                s.binMax = effMin + (i + 1) * (effRange / 5.0f);
                slices.push_back(s);
            }
        }

        // Thêm duy nhất 1 nhóm Outliers vào vị trí cuối cùng (nếu có dữ liệu)
        if (outlierCount > 0.0f) {
            PieSliceInfo s;
            s.weight = outlierCount;
            s.name = "Ngoại lai (Outliers)";
            s.binMin = 0.0f;
            s.binMax = 0.0f;
            slices.push_back(s);
        }
    }

    float totalWeight = 0.0f;
    for (const auto &s : slices) {
        totalWeight += s.weight;
    }

    if (totalWeight <= 0.0f) {
        slices.clear();
        return slices;
    }

    int totalSlices = static_cast<int>(slices.size());
    for (int i = 0; i < totalSlices; ++i) {
        slices[i].arcAngle = (slices[i].weight / totalWeight) * 2.0f * static_cast<float>(M_PI);
    }

    // Ép độ rộng tối thiểu 5.0 độ cho miếng Spikes (hoặc bất kỳ miếng nào có góc < 5.0 độ)
    if (totalSlices > 1) {
        float minArc = 5.0f * static_cast<float>(M_PI) / 180.0f; // 5.0 độ
        for (int i = 0; i < totalSlices; ++i) {
            if (slices[i].arcAngle < minArc) {
                float diff = minArc - slices[i].arcAngle;
                // Tìm lát bánh lớn nhất (khác i) để khấu trừ phần độ dư
                int maxIdx = -1;
                float maxArc = 0.0f;
                for (int j = 0; j < totalSlices; ++j) {
                    if (j != i && slices[j].arcAngle > maxArc) {
                        maxArc = slices[j].arcAngle;
                        maxIdx = j;
                    }
                }
                if (maxIdx != -1 && slices[maxIdx].arcAngle > diff + minArc) {
                    slices[maxIdx].arcAngle -= diff;
                    slices[i].arcAngle = minArc;
                }
            }
        }
    }

    // Đồng bộ góc startAngle và phối màu HSL xoay vòng (Hue Rotation)
    float currentAngle = 0.0f;
    float baseH, baseS, baseL, baseA;
    baseColor.getHslF(&baseH, &baseS, &baseL, &baseA);
    if (baseH < 0.0f) baseH = 0.0f; // Xử lý nếu màu achromatic (trắng/đen/xám)
    if (baseS < 0.25f) baseS = 0.65f; // Đảm bảo độ bão hòa đủ cao để màu xoay vòng nổi bật
    if (baseL < 0.3f) baseL = 0.45f;  // Tránh màu quá tối
    if (baseL > 0.8f) baseL = 0.6f;   // Tránh màu quá sáng

    int mainSliceIndex = 0;
    for (int i = 0; i < totalSlices; ++i) {
        slices[i].startAngle = currentAngle;
        
        // Riêng nhóm Outliers (Spikes): Màu tương phản mạnh nhất với màu chủ đạo (đối diện trên vòng tròn màu sắc +180 độ)
        if (slices[i].name == "Ngoại lai (Outliers)") {
            float oppH = baseH + 0.5f;
            while (oppH >= 1.0f) oppH -= 1.0f;
            // Độ bão hòa cao nhất (s = 1.0) và độ sáng tương phản/nổi bật (l = 0.5) để tăng tính cảnh báo
            QColor oppColor = QColor::fromHslF(oppH, 1.0f, 0.5f, 1.0f);
            slices[i].color = QVector4D(
                static_cast<float>(oppColor.redF()),
                static_cast<float>(oppColor.greenF()),
                static_cast<float>(oppColor.blueF()),
                1.0f);
        } else {
            // Lát đầu tiên dùng gốc (mainSliceIndex == 0), các lát tiếp theo tự động cộng thêm 50 độ vào Hue
            float sliceH = baseH + (static_cast<float>(mainSliceIndex) * (50.0f / 360.0f));
            while (sliceH >= 1.0f) sliceH -= 1.0f;
            while (sliceH < 0.0f) sliceH += 1.0f;

            QColor sliceColor = QColor::fromHslF(sliceH, baseS, baseL, baseA);
            slices[i].color = QVector4D(
                static_cast<float>(sliceColor.redF()),
                static_cast<float>(sliceColor.greenF()),
                static_cast<float>(sliceColor.blueF()),
                1.0f);
            mainSliceIndex++;
        }

        currentAngle += slices[i].arcAngle;
    }

    return slices;
}

void pieChartStrategy::draw(QOpenGLFunctions *f, float time, const QColor &color,
                            const std::vector<DataPoint> &rawData,
                            float minX, float maxX, float minY, float maxY,
                            bool dataDirty, int lineStyle)
{
    Q_UNUSED(time);
    Q_UNUSED(dataDirty);
    Q_UNUSED(lineStyle);

    if (rawData.empty()) return;

    auto slices = computeSlices(rawData, minX, maxX, minY, maxY, m_pieBinMode, color);
    if (slices.empty()) return;

    int sliceCount = static_cast<int>(slices.size());
    if (sliceCount > 12) sliceCount = 12;

    std::vector<float> startAngles(12, 0.0f);
    std::vector<float> arcAngles(12, 0.0f);
    std::vector<QVector4D> sliceColors(12, QVector4D(0, 0, 0, 0));

    for (int i = 0; i < sliceCount; ++i) {
        startAngles[i] = slices[i].startAngle;
        arcAngles[i] = slices[i].arcAngle;
        
        if (i == m_hoveredSlice) {
            // Hiệu ứng Hover: Tăng độ bão hòa (Saturation) và làm sáng nhẹ để trông nổi bật hơn
            QColor col = QColor::fromRgbF(slices[i].color.x(), slices[i].color.y(), slices[i].color.z(), slices[i].color.w());
            float h, s, l, a;
            col.getHslF(&h, &s, &l, &a);
            if (h < 0.0f) h = 0.0f;
            s = std::min(1.0f, s + 0.35f); // Tăng độ bão hòa
            l = std::min(0.85f, l + 0.15f); // Tăng sáng nhẹ để rực rỡ
            QColor hoverCol = QColor::fromHslF(h, s, l, a);
            sliceColors[i] = QVector4D(
                static_cast<float>(hoverCol.redF()),
                static_cast<float>(hoverCol.greenF()),
                static_cast<float>(hoverCol.blueF()),
                1.0f);
        } else {
            sliceColors[i] = slices[i].color;
        }
    }

    program->bind();
    vao.bind();
    program->setUniformValue("u_useMapping", 0);
    program->setUniformValue("u_chartType", 2);

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

    program->setUniformValue("u_scale", QVector2D(scaleX, scaleY));
    program->setUniformValue("u_sliceCount", sliceCount);
    program->setUniformValueArray("u_sliceStartAngles", startAngles.data(), 12, 1);
    program->setUniformValueArray("u_sliceAngles", arcAngles.data(), 12, 1);
    program->setUniformValueArray("u_sliceColors", sliceColors.data(), 12);
    program->setUniformValue("u_hoveredSlice", m_hoveredSlice);
    program->setUniformValue("u_innerRadius", 0.26f); // Lỗ Donut SDF
    program->setUniformValue("u_outerRadius", 0.56f);
    program->setUniformValue("u_cornerRadius", 0.025f); // Bo tròn góc
    program->setUniformValue("u_gap", 0.035f); // Khoảng cách giữa các lát
    program->setUniformValue("u_explodeOffset", 0.06f); // Nhô ra khi hover

    vboQuad.bind();
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    program->enableAttributeArray(0);

    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    f->glDisable(GL_BLEND);

    vao.release();
    program->release();
}
