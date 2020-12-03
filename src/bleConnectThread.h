#ifndef BLECONNECTTHREAD_H
#define BLECONNECTTHREAD_H

#include <QObject>
#include <QThread>

class BLEConnectThread : public QThread
{
    Q_OBJECT
public:
    void run() override;

signals:

};

#endif // BLECONNECTTHREAD_H
