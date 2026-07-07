#include "chartaxiscalculator.h"
#include <QVariantMap>
#include <QString>
#include <cmath>

float ChartAxisCalculator::calculateGridStep(float range)
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

QVariantList ChartAxisCalculator::calculateTicks(float minVal, float maxVal)
{
    QVariantList list;
    float range = maxVal - minVal;
    if (range <= 0.0f) return list;

    float step = calculateGridStep(range);

    // Tìm tick bắt đầu lớn hơn hoặc bằng minVal và chia hết cho step
    float firstTick = std::ceil(minVal / step) * step;

    for (float val = firstTick; val <= maxVal; val += step) {
        if (val < minVal - 1e-5f || val > maxVal + 1e-5f) continue;

        QVariantMap tick;
        tick["value"] = QString::number(val, 'g', 6);
        tick["position"] = (val - minVal) / range;
        tick["val"] = val;
        list.append(tick);
    }
    return list;
}
