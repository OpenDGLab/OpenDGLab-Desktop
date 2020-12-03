#include "mainWindowController.h"
#include "global.h"
MainWindowController::MainWindowController(QObject *parent) : QObject(parent)
{
    connectThread.start();
    autoWaveChangerThread.start();
}

MainWindowController::~MainWindowController() {
    autoWaveChangerThread.quit();
    connectThread.requestInterruption();
    connectThread.wait();
}

void MainWindowController::startAllBoost(int boost)
{
    for (auto device: Global::dglabList) {
        if (device->getState() == DeviceStateEnum::DeviceState::READY || device->getState() == DeviceStateEnum::DeviceState::READY_REMOTEMANAGED)
            device->startGlobalBoost(boost);
    }
}

void MainWindowController::stopAllBoost()
{
    for (auto device: Global::dglabList) {
        if (device->getState() == DeviceStateEnum::DeviceState::READY || device->getState() == DeviceStateEnum::DeviceState::READY_REMOTEMANAGED)
            device->stopGlobalBoost();
    }
}
