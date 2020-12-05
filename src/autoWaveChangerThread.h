#ifndef AUTOWAVECHANGERTHREAD_H
#define AUTOWAVECHANGERTHREAD_H

#include <QThread>
#include <QTimer>
#include <QObject>

class AutoWaveChangerThread : public QThread
{
    Q_OBJECT
public:
    explicit AutoWaveChangerThread(QObject *parent = nullptr);
    ~AutoWaveChangerThread() override;
    void run() override;
private:
    QTimer* timer{};
};

#endif // AUTOWAVECHANGERTHREAD_H
