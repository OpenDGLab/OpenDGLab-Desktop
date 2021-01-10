//
// Created by max_3 on 2021/1/11.
//

#include "autoRemoteProtocolPinger.h"
#include <QTimer>
#include "global.h"

#include <iostream>
AutoRemoteProtocolPinger::AutoRemoteProtocolPinger(QObject* parent): QThread(parent)
{
    timer = new QTimer();
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, [](){
        for(auto r: Global::remoteList) {
            r->sendPing();
        }
    });
    connect(this, &QThread::finished, timer, &QTimer::stop);
    timer->start();
}

AutoRemoteProtocolPinger::~AutoRemoteProtocolPinger()
{
    delete timer;
}

void AutoRemoteProtocolPinger::run()
{
    this->exec();
}
