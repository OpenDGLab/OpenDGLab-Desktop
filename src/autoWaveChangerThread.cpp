#include "autoWaveChangerThread.h"
#include <QTimer>
#include "global.h"
AutoWaveChangerThread::AutoWaveChangerThread(QObject* parent): QThread(parent)
{

}

AutoWaveChangerThread::~AutoWaveChangerThread()
{
    timer = new QTimer();
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, [](){
        for (auto device: Global::dglabList) {
            if (device->getState() == DeviceStateEnum::DeviceState::READY || device->getState() == DeviceStateEnum::DeviceState::READY_REMOTEMANAGED) {
                device->getUiDeviceOperator()->checkIfNeedChangeWave();
            }
        }
    });
    connect(this, &QThread::finished, timer, &QTimer::stop);
    timer->start();
    this->exec();
}

void AutoWaveChangerThread::run()
{
    delete timer;
}
