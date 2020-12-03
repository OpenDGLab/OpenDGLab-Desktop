#include "scanDeviceController.h"
#include "global.h"
#include <QDebug>
#include <QMutexLocker>
ScanDeviceController::ScanDeviceController(QObject *parent) : QObject(parent)
{
    connect(Global::bleScanAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &ScanDeviceController::deviceScanFound);
    connect(Global::bleScanAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error), this, &ScanDeviceController::deviceScanError);
    connect(Global::bleScanAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &ScanDeviceController::deviceScanFinished);
}

ScanDeviceController::~ScanDeviceController() {
    stopScan();
}
void ScanDeviceController::startScan() {
    Global::bleScanAgent->start();
}

void ScanDeviceController::stopScan() {
    Global::bleScanAgent->stop();
}

void ScanDeviceController::deviceScanFound(const QBluetoothDeviceInfo& info) {
    QMutexLocker locker(&Global::mutex);
    bool isSaved = false;
    for (auto savedDevice: (Global::dglabList)) {
        if(savedDevice->getDeviceAddressOrUuid() ==
        #ifdef Q_OS_MACOS
            info.deviceUuid()
        #else
            info.address()
        #endif
                .toString()
                ) {
            isSaved = true;
        }
    }
    if (info.name() == Global::deviceName && !isSaved) {
        stopScan();
        DGLabDevice *device = new DGLabDevice(info);
        Global::dglabList.append(device);
        emit eventScanFound();
    }
}

void ScanDeviceController::deviceScanError() {
    emit eventScanError();
}

void ScanDeviceController::deviceScanFinished() {
    emit eventScanFinished();
}
