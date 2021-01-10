//
// Created by max_3 on 2021/1/11.
//

#ifndef OPENDGLAB_DESKTOP_AUTOREMOTEPROTOCOLPINGER_H
#define OPENDGLAB_DESKTOP_AUTOREMOTEPROTOCOLPINGER_H

#include <QThread>
#include <QTimer>
#include <QObject>

class AutoRemoteProtocolPinger : public QThread
{
    Q_OBJECT
public:
    explicit AutoRemoteProtocolPinger(QObject *parent = nullptr);
    ~AutoRemoteProtocolPinger() override;
    void run() override;
private:
    QTimer* timer{};

};


#endif //OPENDGLAB_DESKTOP_AUTOREMOTEPROTOCOLPINGER_H
