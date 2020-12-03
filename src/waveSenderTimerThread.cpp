#include "waveSenderTimerThread.h"
#include "dglabDevice.h"
WaveSenderTimerThread::WaveSenderTimerThread(QObject *parent):
    QThread(parent)
{
    connect(this, &QThread::finished, this, &QThread::deleteLater);
}

void WaveSenderTimerThread::run()
{
    timer = new QTimer();
    timer->setInterval(200);
    connect(timer, &QTimer::timeout, parent(), [this](){
        reinterpret_cast<DGLabDevice*>(parent())->waveSender();
    });
    connect(this, &QThread::finished, timer, &QTimer::stop);
    timer->start();
    this->exec();
}

WaveSenderTimerThread::~WaveSenderTimerThread() {
    delete timer;
}
