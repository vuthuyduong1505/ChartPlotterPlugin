#ifndef ONLINESTREAM_H
#define ONLINESTREAM_H

#include <QObject>
#include <QTimer>

// Mô phỏng nguồn dữ liệu real-time (online mode) — đẩy điểm mới vào DataManager theo chu kỳ.
class OnlineStream : public QObject
{
    Q_OBJECT
public:
    static OnlineStream *instance();

    Q_INVOKABLE void start(int intervalMs = 100);
    Q_INVOKABLE void stop();
    Q_INVOKABLE void pauseStream();
    Q_INVOKABLE void resumeStream();
    bool isRunning() const { return m_timer.isActive(); }

signals:
    void runningChanged();

private:
    explicit OnlineStream(QObject *parent = nullptr);
    void onTick();

    QTimer m_timer;
    float m_counter = 0.0f;
};

#endif // ONLINESTREAM_H
