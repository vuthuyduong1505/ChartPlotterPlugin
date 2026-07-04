#include "onlinestream.h"
#include "datamanager.h"
#include <QRandomGenerator>
#include <QtMath>

namespace {
constexpr int kMaxOnlinePoints = 400;
}

OnlineStream *OnlineStream::instance()
{
    static OnlineStream stream;
    return &stream;
}

OnlineStream::OnlineStream(QObject *parent) : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &OnlineStream::onTick);
}

void OnlineStream::start(int intervalMs)
{
    if (m_timer.isActive())
        return;

    m_counter = 0.0f;
    m_timer.start(qMax(intervalMs, 16));
    emit runningChanged();
}

void OnlineStream::stop()
{
    if (!m_timer.isActive())
        return;

    m_timer.stop();
    emit runningChanged();
}

void OnlineStream::onTick()
{
    auto *dm = DataManager::instance();

    // Giả lập sensor: dao động sin + nhiễu ngẫu nhiên nhẹ
    const float noise = QRandomGenerator::global()->bounded(-100, 101) / 20.0f;
    const float y = 50.0f + 25.0f * qSin(m_counter * 0.12f) + noise;

    dm->addData(m_counter, y);
    m_counter += 1.0f;

    // Giữ cửa sổ trượt để biểu đồ online mượt và không phình bộ nhớ
    dm->keepLastN(kMaxOnlinePoints);
}
