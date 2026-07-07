#include "charthittester.h"
#include "datamanager.h"
#include "dataprocessor.h"
#include "strategies/piechartstrategy.h"
#include <QtMath>
#include <algorithm>

QVariantMap ChartHitTester::getNearestDataPoint(float mouseX, float mouseY, 
                                                float screenWidth, float screenHeight,
                                                int chartType, int pieBinMode, 
                                                const QColor &chartColor,
                                                const ViewportManager &viewport, 
                                                int &hoveredSlice, bool &sliceChanged)
{
    QVariantMap result;
    result["valid"] = false;
    result["dataX"] = 0.0f;
    result["dataY"] = 0.0f;
    result["screenX"] = 0.0f;
    result["screenY"] = 0.0f;
    result["isPie"] = false;
    result["percent"] = 0.0f;
    result["sliceName"] = "";

    sliceChanged = false;

    if (screenWidth <= 0.0f || screenHeight <= 0.0f) {
        return result;
    }

    auto dm = DataManager::instance();
    if (dm->isEmpty()) {
        return result;
    }

    // Bước 1: Ánh xạ ngược mouseX sang giá trị targetDataX
    float xGL = (mouseX / screenWidth) * 2.0f - 1.0f;
    float mapMinX = -1.0f;
    float mapMaxX = 1.0f;
    float mapMinY = -1.0f;
    float mapMaxY = 1.0f;

    float u_minX = viewport.viewMinX();
    float u_maxX = viewport.viewMaxX();

    float denX = (u_maxX - u_minX) == 0.0f ? 0.001f : (u_maxX - u_minX);
    float targetDataX = u_minX + (xGL - mapMinX) * denX / (mapMaxX - mapMinX);

    // Ràng buộc targetDataX trong khoảng dữ liệu thực tế hiện có để tránh ngoại suy lỗi
    targetDataX = std::max(dm->firstPoint().x, std::min(dm->lastPoint().x, targetDataX));

    DataPoint p1, p2;
    if (chartType != 2) {
        std::pair<DataPoint, DataPoint> adj = dm->findAdjacentPoints(targetDataX);
        p1 = adj.first;
        p2 = adj.second;
    }

    if (chartType == 0) {
        // --- LINE CHART: Nội suy tuyến tính Y ---
        float interpolatedDataY = p1.y;
        if (p2.x - p1.x != 0.0f) {
            interpolatedDataY = p1.y + (p2.y - p1.y) * ((targetDataX - p1.x) / (p2.x - p1.x));
        }

        // Ánh xạ xuôi (Forward Mapping) để chuyển (targetDataX, interpolatedDataY) về pixel màn hình
        float snappedXGL = (mapMaxX - mapMinX) * (targetDataX - u_minX) / denX + mapMinX;

        float u_minY = viewport.viewMinY();
        float u_maxY = viewport.viewMaxY();
        if (qAbs(u_maxY - u_minY) < 0.0001f) {
            u_maxY += 1.0f;
            u_minY -= 1.0f;
        }

        float denY = (u_maxY - u_minY) == 0.0f ? 0.001f : (u_maxY - u_minY);
        float snappedYGL = (mapMaxY - mapMinY) * (interpolatedDataY - u_minY) / denY + mapMinY;

        float lineScreenY = (1.0f - snappedYGL) * 0.5f * screenHeight;

        float distanceY = qAbs(mouseY - lineScreenY);

        if (distanceY <= 15.0f) {
            result["valid"] = true;
            result["dataX"] = targetDataX;
            result["dataY"] = interpolatedDataY;
            result["screenX"] = mouseX; // Bám trơn tru theo con trỏ chuột
            result["screenY"] = lineScreenY; // Hút thẳng vào nét vẽ
        } else {
            result["valid"] = false;
        }
    } else if (chartType == 1) {
        // --- BAR CHART: PIXEL-PERFECT HIT TESTING ---
        float u_minY = viewport.viewMinY();
        float u_maxY = viewport.viewMaxY();
        if (qAbs(u_maxY - u_minY) < 0.0001f) {
            u_maxY += 1.0f;
            u_minY -= 1.0f;
        }
        float denY = (u_maxY - u_minY) == 0.0f ? 0.001f : (u_maxY - u_minY);

        // 1. Tính bề rộng chuẩn của cột trên màn hình pixel (Đồng bộ với DataProcessor và tỷ lệ Zoom)
        float barWidth = DataProcessor::calculateBarWidth(dm->getData(), viewport.dataMinX(), viewport.dataMaxX());
        float pixelHalfWidth = (barWidth / denX) * screenWidth;
        
        // 2. Tính tọa độ Y của đáy cột (zeroScreenY)
        float zeroYGL = (mapMaxY - mapMinY) * (0.0f - u_minY) / denY + mapMinY;
        float zeroScreenY = (1.0f - zeroYGL) * 0.5f * screenHeight;

        // 3. Hàm Lambda phụ trợ: Ánh xạ 1 điểm ra màn hình và kiểm tra chuột có nằm trong hình chữ nhật của nó không
        auto checkHit = [&](const DataPoint& p, float& screenX, float& screenY) -> bool {
            // Ánh xạ X
            float pXGL = (mapMaxX - mapMinX) * (p.x - u_minX) / denX + mapMinX;
            screenX = (pXGL + 1.0f) * 0.5f * screenWidth;
            // Ánh xạ Y
            float pYGL = (mapMaxY - mapMinY) * (p.y - u_minY) / denY + mapMinY;
            screenY = (1.0f - pYGL) * 0.5f * screenHeight;
            
            // Kiểm tra chuột có nằm trong vùng Pixel Rectangle của cột (mở rộng 1px chống nhiễu)
            bool withinX = (mouseX >= screenX - pixelHalfWidth - 1.0f) && (mouseX <= screenX + pixelHalfWidth + 1.0f);
            float minYBound = qMin(screenY, zeroScreenY);
            float maxYBound = qMax(screenY, zeroScreenY);
            bool withinY = (mouseY >= minYBound) && (mouseY <= maxYBound);
            
            return withinX && withinY;
        };

        // 4. Áp dụng kiểm tra trực tiếp Hitbox cho cả 2 điểm p1 và p2
        float sX1 = 0, sY1 = 0, sX2 = 0, sY2 = 0;
        bool hit1 = checkHit(p1, sX1, sY1);
        bool hit2 = checkHit(p2, sX2, sY2);

        if (hit1) {
            result["valid"] = true;
            result["dataX"] = p1.x;
            result["dataY"] = p1.y;
            result["screenX"] = sX1;
            result["screenY"] = sY1;
        } else if (hit2) {
            result["valid"] = true;
            result["dataX"] = p2.x;
            result["dataY"] = p2.y;
            result["screenX"] = sX2;
            result["screenY"] = sY2;
        } else {
            result["valid"] = false;
        }
    } else if (chartType == 2) {
        // --- PIE / DONUT CHART: HIT-TESTING ĐỒNG BỘ VỚI OPENGL SDF ---
        float cx = screenWidth / 2.0f;
        float cy = screenHeight / 2.0f;
        float minDim = qMin(screenWidth, screenHeight) / 2.0f;
        float outerRadius = 0.56f * minDim;
        float innerRadius = 0.26f * minDim; // Lỗ donut SDF

        float dx = mouseX - cx;
        float dy = cy - mouseY;
        float dist = std::sqrt(dx * dx + dy * dy);

        int hitSlice = -1;
        if (dist >= innerRadius && dist <= outerRadius) {
            const auto data = dm->getData();
            float minX, maxX, minY, maxY;
            DataProcessor::calculateBounds(data, minX, maxX, minY, maxY);

            auto slices = pieChartStrategy::computeSlices(data, minX, maxX, minY, maxY, pieBinMode, chartColor);
            
            float totalSum = 0.0f;
            for (const auto& s : slices) {
                totalSum += s.weight;
            }

            if (totalSum > 0.0f) {
                float mouseAngle = std::atan2(dy, dx);
                if (mouseAngle < 0.0f) mouseAngle += 2.0f * static_cast<float>(M_PI);

                for (size_t i = 0; i < slices.size(); ++i) {
                    float sliceAngle = slices[i].arcAngle;
                    if (sliceAngle > 0.0f && mouseAngle >= slices[i].startAngle && mouseAngle < slices[i].startAngle + sliceAngle) {
                        result["valid"] = true;
                        result["isPie"] = true;
                        result["sliceName"] = slices[i].name;
                        result["percent"] = (slices[i].weight / totalSum) * 100.0f;
                        result["dataX"] = 0.0f;
                        result["dataY"] = slices[i].weight;
                        result["screenX"] = mouseX;
                        result["screenY"] = mouseY;
                        result["currentBinMin"] = slices[i].binMin;
                        result["currentBinMax"] = slices[i].binMax;
                        hitSlice = static_cast<int>(i);
                        break;
                    }
                }
            }
        }

        if (hoveredSlice != hitSlice) {
            hoveredSlice = hitSlice;
            sliceChanged = true;
        }
    }

    if (chartType != 2 && hoveredSlice != -1) {
        hoveredSlice = -1;
        sliceChanged = true;
    }

    return result;
}
