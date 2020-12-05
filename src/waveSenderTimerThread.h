#ifndef WAVESENDERTIMERTHREAD_H
#define WAVESENDERTIMERTHREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>
class WaveSenderTimerThread : public QThread
{
    Q_OBJECT
public:
    explicit WaveSenderTimerThread(QObject *parent = nullptr);
    ~WaveSenderTimerThread() override;
    void run() override;
private:
    QTimer* timer{};
};

#endif // WAVESENDERTIMERTHREAD_H
