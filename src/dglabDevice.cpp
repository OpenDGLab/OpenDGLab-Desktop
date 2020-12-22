#include "dglabDevice.h"
#include "global.h"
#include <QTimer>
DGLabDevice::DGLabDevice(const QBluetoothDeviceInfo &_deviceInfo, QObject *parent) :
    QObject(parent),
    deviceInfo(_deviceInfo),
    openDgLab(libopendglab_symbols()->kotlin.root.OpenDGLab.OpenDGLab())
{
    controller = QLowEnergyController::createCentral(deviceInfo, this);
    controller->setRemoteAddressType(QLowEnergyController::PublicAddress);
    id = QString(QCryptographicHash::hash(getDeviceAddressOrUuid().toUtf8(),QCryptographicHash::Sha256).toHex()).right(6);
    uiDeviceItem = new DeviceItem(getID(), getDeviceAddressOrUuid(), Global::deviceName);
    uiDeviceOperator = new DeviceOperator(getID());
    uiDeviceOperator->setEnabled(false);
    connect(controller, &QLowEnergyController::connected, this, &DGLabDevice::deviceConnected);
    connect(controller, &QLowEnergyController::disconnected, this, &DGLabDevice::deviceDisconnected);
    connect(this, &DGLabDevice::powerUpdate, uiDeviceOperator, &DeviceOperator::setPower);
    connect(uiDeviceOperator, &DeviceOperator::changeWave, this, &DGLabDevice::setWave);
    connect(uiDeviceOperator, &DeviceOperator::startBoost, this, &DGLabDevice::startChannelBoost);
    connect(uiDeviceOperator, &DeviceOperator::stopBoost, this, &DGLabDevice::stopChannelBoost);
}

DGLabDevice::~DGLabDevice()
{
    if (waveSenderTimerThread) {
        waveSenderTimerThread->quit();
        //waveSenderTimerThread->wait();
        waveSenderTimerThread = nullptr;
    }
    SKIPNEXTQWARNING(SKIPNEXTERROR(controller->disconnectFromDevice()));
}

QString DGLabDevice::getID()
{
    return id;
}

QString DGLabDevice::getDeviceAddressOrUuid()
{
    QString address =
    #ifdef Q_OS_MACOS
            deviceInfo.deviceUuid()
    #else
            deviceInfo.address()
    #endif
            .toString();
    return address;
}

QLowEnergyController* DGLabDevice::getBLEController()
{
    return controller;
}

void DGLabDevice::startGlobalBoost(int boost)
{
    if (globalBoost != 0) return;
    globalBoost = boost;
    const QLowEnergyCharacteristic abpower = eStimService->characteristic(QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::abpower)));
    libopendglab_ExportedSymbols* libopendglab = libopendglab_symbols();
    auto eStim = libopendglab->kotlin.root.OpenDGLab.get_eStimStatus(openDgLab);
    auto power = libopendglab->kotlin.root.OpenDGLab.EStimStatus.get_abPower(eStim);
    libopendglab_kref_OpenDGLab_WriteBLE dataKt;
    auto afboostA = powerA + boost;
    auto afboostB = powerB + boost;
    if (afboostA > 274) afboostA = 274;
    if (afboostB > 274) afboostB = 274;
    dataKt = libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.setABPower(power, afboostA, afboostB);
    auto dataBytes = libopendglab->kotlin.root.OpenDGLab.WriteBLE.get_data(dataKt);
    auto dataSize = libopendglab->kotlin.root.getByteArraySize(dataBytes);
    auto data = QByteArray(dataSize, Qt::Initialization::Uninitialized);
    libopendglab->kotlin.root.toNativeByteArray(data.data(), dataBytes);
    eStimService->writeCharacteristic(abpower, data, QLowEnergyService::WriteWithoutResponse);
}

void DGLabDevice::stopGlobalBoost()
{
    const QLowEnergyCharacteristic abpower = eStimService->characteristic(QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::abpower)));
    libopendglab_ExportedSymbols* libopendglab = libopendglab_symbols();
    auto eStim = libopendglab->kotlin.root.OpenDGLab.get_eStimStatus(openDgLab);
    auto power = libopendglab->kotlin.root.OpenDGLab.EStimStatus.get_abPower(eStim);
    libopendglab_kref_OpenDGLab_WriteBLE dataKt;
    auto afboostA = powerA - globalBoost;
    auto afboostB = powerB - globalBoost;
    if (afboostA < 0) afboostA = 0;
    if (afboostB < 0) afboostB = 0;
    dataKt = libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.setABPower(power, afboostA, afboostB);
    auto dataBytes = libopendglab->kotlin.root.OpenDGLab.WriteBLE.get_data(dataKt);
    auto dataSize = libopendglab->kotlin.root.getByteArraySize(dataBytes);
    auto data = QByteArray(dataSize, Qt::Initialization::Uninitialized);
    libopendglab->kotlin.root.toNativeByteArray(data.data(), dataBytes);
    eStimService->writeCharacteristic(abpower, data, QLowEnergyService::WriteWithoutResponse);
    globalBoost = 0;
}

DeviceStateEnum::DeviceState DGLabDevice::getState()
{
    return uiDeviceItem->getState();
}

void DGLabDevice::waveSender()
{
    if (uiDeviceItem->getState() == DeviceStateEnum::DeviceState::READY || uiDeviceItem->getState() == DeviceStateEnum::DeviceState::READY_REMOTEMANAGED) {
        libopendglab_ExportedSymbols* libopendglab = libopendglab_symbols();
        auto eStimStatus = libopendglab->kotlin.root.OpenDGLab.get_eStimStatus(openDgLab);
        auto wave = libopendglab->kotlin.root.OpenDGLab.EStimStatus.get_wave(eStimStatus);
        if (powerA > 0) {
            //WaveA
            auto waveCenterA = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.getWaveCenterA(wave);
            auto hasWave = libopendglab->kotlin.root.WaveCenter.toDGWaveGen(waveCenterA);
            if (QString::fromUtf8(hasWave) == "") {
                //Custom Send
                QMutexLocker locker(&customWaveAMutex);
                if (!customWaveA.isEmpty()) {
                    auto qBytes = customWaveA.takeFirst();
                    const QLowEnergyCharacteristic wave = eStimService->characteristic(QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::waveA)));
                    eStimService->writeCharacteristic(wave, qBytes, QLowEnergyService::WriteWithoutResponse);
                }
            } else {
                auto waveBytes = libopendglab->kotlin.root.WaveCenter.waveTick(waveCenterA);
                auto waveBytesSize = libopendglab->kotlin.root.getByteArraySize(waveBytes);
                auto qBytes = QByteArray(waveBytesSize, Qt::Initialization::Uninitialized);
                libopendglab->kotlin.root.toNativeByteArray(qBytes.data(), waveBytes);
                const QLowEnergyCharacteristic wave = eStimService->characteristic(QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::waveA)));
                eStimService->writeCharacteristic(wave, qBytes, QLowEnergyService::WriteWithoutResponse);
            }
            libopendglab->DisposeString(hasWave);
        } else {
            auto waveCenterA = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.getWaveCenterA(wave);
            auto hasWave = libopendglab->kotlin.root.WaveCenter.toDGWaveGen(waveCenterA);
            if (QString::fromUtf8(hasWave) == "") {
                QMutexLocker locker(&customWaveAMutex);
                if (!customWaveA.isEmpty()) customWaveA.empty();
            }
        }
        if (powerB > 0){
            //WaveB
            auto waveCenterB = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.getWaveCenterB(wave);
            auto hasWave = libopendglab->kotlin.root.WaveCenter.toDGWaveGen(waveCenterB);
            if (QString::fromUtf8(hasWave) == "") {
                QMutexLocker locker(&customWaveBMutex);
                if (!customWaveB.isEmpty()) {
                    auto qBytes = customWaveB.takeFirst();
                    const QLowEnergyCharacteristic wave = eStimService->characteristic(QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::waveB)));
                    eStimService->writeCharacteristic(wave, qBytes, QLowEnergyService::WriteWithoutResponse);
                }
            } else {
                auto waveBytes = libopendglab->kotlin.root.WaveCenter.waveTick(waveCenterB);
                auto waveBytesSize = libopendglab->kotlin.root.getByteArraySize(waveBytes);
                auto qBytes = QByteArray(waveBytesSize, Qt::Initialization::Uninitialized);
                libopendglab->kotlin.root.toNativeByteArray(qBytes.data(), waveBytes);
                const QLowEnergyCharacteristic wave = eStimService->characteristic(QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::waveB)));
                eStimService->writeCharacteristic(wave, qBytes, QLowEnergyService::WriteWithoutResponse);
            }
            libopendglab->DisposeString(hasWave);
        } else {
            auto waveCenterB = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.getWaveCenterB(wave);
            auto hasWave = libopendglab->kotlin.root.WaveCenter.toDGWaveGen(waveCenterB);
            if (QString::fromUtf8(hasWave) == "") {
                QMutexLocker locker(&customWaveBMutex);
                if (!customWaveB.isEmpty()) customWaveB.empty();
            }
        }
    }
}

void DGLabDevice::setWave(DeviceStateEnum::DeviceChannel channel, QString value)
{
    libopendglab_ExportedSymbols* libopendglab = libopendglab_symbols();
    auto eStimState = libopendglab->kotlin.root.OpenDGLab.get_eStimStatus(openDgLab);
    auto wave = libopendglab->kotlin.root.OpenDGLab.EStimStatus.get_wave(eStimState);
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
    {
        auto waveCenter = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.getWaveCenterA(wave);
        auto cWave = Global::basicWaveNameList.contains(value);
        if (cWave) {
            auto waveA = libopendglab->kotlin.root.WaveCenter.Companion.getBasicWave(libopendglab->kotlin.root.WaveCenter.Companion._instance(), value.toUtf8().data());
            auto convertWaveA = libopendglab->kotlin.root.convertBasicWaveDataToBasicWave(waveA);
            libopendglab->kotlin.root.WaveCenter.selectWave(waveCenter, convertWaveA);
        } else {
            cWave = Global::touchWaveNameList.contains(value);
            if (cWave) {
                auto waveA = libopendglab->kotlin.root.WaveCenter.Companion.getTouchWave(libopendglab->kotlin.root.WaveCenter.Companion._instance(), value.toUtf8().data());
                auto convertWaveA = libopendglab->kotlin.root.convertTouchWaveDataToBasicWave(waveA);
                libopendglab->kotlin.root.WaveCenter.selectWave(waveCenter, convertWaveA);
            } else {
                libopendglab->kotlin.root.WaveCenter.selectWave(waveCenter, libopendglab->kotlin.root.createNullBasicWave());
            }
        }
    }
    break;
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
    {
        auto waveCenter = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.getWaveCenterB(wave);
        auto cWave = Global::basicWaveNameList.contains(value);
        if (cWave) {
            auto waveB = libopendglab->kotlin.root.WaveCenter.Companion.getBasicWave(libopendglab->kotlin.root.WaveCenter.Companion._instance(), value.toUtf8().data());
            auto convertWaveB = libopendglab->kotlin.root.convertBasicWaveDataToBasicWave(waveB);
            libopendglab->kotlin.root.WaveCenter.selectWave(waveCenter, convertWaveB);
        } else {
            cWave = Global::touchWaveNameList.contains(value);
            if (cWave) {
                auto waveB = libopendglab->kotlin.root.WaveCenter.Companion.getTouchWave(libopendglab->kotlin.root.WaveCenter.Companion._instance(), value.toUtf8().data());
                auto convertWaveB = libopendglab->kotlin.root.convertTouchWaveDataToBasicWave(waveB);
                libopendglab->kotlin.root.WaveCenter.selectWave(waveCenter, convertWaveB);
            } else {
                libopendglab->kotlin.root.WaveCenter.selectWave(waveCenter, libopendglab->kotlin.root.createNullBasicWave());
            }
        }
    }
    break;
    }
}

void DGLabDevice::startChannelBoost(DeviceStateEnum::DeviceChannel channel, int boost)
{
    const QLowEnergyCharacteristic abpower = eStimService->characteristic(QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::abpower)));
    libopendglab_ExportedSymbols* libopendglab = libopendglab_symbols();
    auto eStim = libopendglab->kotlin.root.OpenDGLab.get_eStimStatus(openDgLab);
    auto power = libopendglab->kotlin.root.OpenDGLab.EStimStatus.get_abPower(eStim);
    libopendglab_kref_OpenDGLab_WriteBLE dataKt;
    int afboost;
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
        boostA = boost;
        afboost = powerA + boost;
        if (afboost > 274) afboost = 274;
        dataKt = libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.setABPower(power, afboost, powerB);
        break;
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
        boostB = boost;
        afboost = powerB + boost;
        if (afboost > 274) afboost = 274;
        dataKt = libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.setABPower(power, powerA, afboost);
        break;
    }
    auto dataBytes = libopendglab->kotlin.root.OpenDGLab.WriteBLE.get_data(dataKt);
    auto dataSize = libopendglab->kotlin.root.getByteArraySize(dataBytes);
    auto data = QByteArray(dataSize, Qt::Initialization::Uninitialized);
    libopendglab->kotlin.root.toNativeByteArray(data.data(), dataBytes);
    eStimService->writeCharacteristic(abpower, data, QLowEnergyService::WriteWithoutResponse);
}

void DGLabDevice::stopChannelBoost(DeviceStateEnum::DeviceChannel channel, int boost)
{
    const QLowEnergyCharacteristic abpower = eStimService->characteristic(QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::abpower)));
    libopendglab_ExportedSymbols* libopendglab = libopendglab_symbols();
    auto eStim = libopendglab->kotlin.root.OpenDGLab.get_eStimStatus(openDgLab);
    auto power = libopendglab->kotlin.root.OpenDGLab.EStimStatus.get_abPower(eStim);
    libopendglab_kref_OpenDGLab_WriteBLE dataKt;
    int afboost;
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
        boostA = 0;
        afboost = powerA - boost;
        if (afboost < 0) afboost = 0;
        dataKt = libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.setABPower(power, afboost, powerB);
        break;
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
        boostB = 0;
        afboost = powerB - boost;
        if (afboost < 0) afboost = 0;
        dataKt = libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.setABPower(power, powerA, afboost);
        break;
    }
    auto dataBytes = libopendglab->kotlin.root.OpenDGLab.WriteBLE.get_data(dataKt);
    auto dataSize = libopendglab->kotlin.root.getByteArraySize(dataBytes);
    auto data = QByteArray(dataSize, Qt::Initialization::Uninitialized);
    libopendglab->kotlin.root.toNativeByteArray(data.data(), dataBytes);
    eStimService->writeCharacteristic(abpower, data, QLowEnergyService::WriteWithoutResponse);
}

bool DGLabDevice::isShownOnUi()
{
    return shownOnUi;
}
void DGLabDevice::setShownOnUi()
{
    shownOnUi = true;
}
DeviceItem* DGLabDevice::getUiDeviceItem() {
    return uiDeviceItem;
}
DeviceOperator* DGLabDevice::getUiDeviceOperator() {
    return uiDeviceOperator;
}

void DGLabDevice::deviceConnected()
{
    uiDeviceItem->changeState(DeviceStateEnum::DeviceState::CONNECTED);
    uiDeviceItem->changeBattery(-1);
    uiDeviceItem->update();
    waveSenderTimerThread = new WaveSenderTimerThread(this);
    connect(controller, &QLowEnergyController::discoveryFinished, this, &DGLabDevice::deviceServiceDiscoverFinished);
    this->controller->discoverServices();
}
void DGLabDevice::deviceDisconnected()
{
    if (waveSenderTimerThread) {
        waveSenderTimerThread->quit();
        //waveSenderTimerThread->wait();
        waveSenderTimerThread = nullptr;
    }
    if (uiDeviceItem) {
        uiDeviceItem->changeState(DeviceStateEnum::DeviceState::UNCONNECTED);
        uiDeviceItem->changeBattery(0);
        uiDeviceItem->update();
    }
    if (remoteLocked) {
        // 通知锁定客户端设备已经释放
        for (auto r: Global::remoteList) {
            r->sendDeviceReset(getID());
        }
    }
    remoteLocked = false;
    uiDeviceOperator->setAutoChange(DeviceStateEnum::DeviceChannel::CHANNEL_A, false);
    uiDeviceOperator->setAutoChange(DeviceStateEnum::DeviceChannel::CHANNEL_B, false);
}
void DGLabDevice::deviceServiceDiscoverFinished()
{
    batteryService = controller->createServiceObject(QBluetoothUuid(QUuid(Global::DGLabServices::DeviceStatus::service)),this);
    if (!batteryService) {
        qDebug() << "Not Found Battery Service";
        return;
    }
    connect(batteryService, qOverload<QLowEnergyService::ServiceState>(&QLowEnergyService::stateChanged), this, &DGLabDevice::deviceBatteryServiceStateChanged);
    connect(batteryService, qOverload<const QLowEnergyCharacteristic&, const QByteArray &>(&QLowEnergyService::characteristicChanged), this, &DGLabDevice::deviceBatteryCharacteristicArrived);
    connect(batteryService, qOverload<const QLowEnergyCharacteristic&, const QByteArray &>(&QLowEnergyService::characteristicRead), this, &DGLabDevice::deviceBatteryCharacteristicArrived);
    eStimService = controller->createServiceObject(QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::service)),this);
    if (!eStimService) {
        qDebug() << "Not Found EStim Service";
        return;
    }
    connect(eStimService, qOverload<QLowEnergyService::ServiceState>(&QLowEnergyService::stateChanged), this, &DGLabDevice::deviceEStimServiceStateChanged);
    connect(eStimService, qOverload<const QLowEnergyCharacteristic&, const QByteArray &>(&QLowEnergyService::characteristicChanged), this, &DGLabDevice::deviceEStimCharacteristicArrived);
    connect(eStimService, qOverload<const QLowEnergyCharacteristic&, const QByteArray &>(&QLowEnergyService::characteristicRead), this, &DGLabDevice::deviceEStimCharacteristicArrived);

    //Qt BLE Bug Workaround https://bugreports.qt.io/browse/QTBUG-78488
    QTimer::singleShot(0, [this] () {
        batteryService->discoverDetails();
        eStimService->discoverDetails();
    });

    //Connect UI Event
    connect(uiDeviceOperator, &DeviceOperator::changePower, this, &DGLabDevice::changePower);
}
void DGLabDevice::changePower(int levelA, int levelB) {
    if (levelA < 0) levelA = 0;
    if (levelB < 0) levelB = 0;
    if (levelA > 274) levelA = 274;
    if (levelB > 274) levelB = 274;
    const QLowEnergyCharacteristic abpower = eStimService->characteristic(QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::abpower)));
    libopendglab_ExportedSymbols* libopendglab = libopendglab_symbols();
    auto eStim = libopendglab->kotlin.root.OpenDGLab.get_eStimStatus(openDgLab);
    auto power = libopendglab->kotlin.root.OpenDGLab.EStimStatus.get_abPower(eStim);
    auto dataKt = libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.setABPower(power, levelA, levelB);
    auto dataBytes = libopendglab->kotlin.root.OpenDGLab.WriteBLE.get_data(dataKt);
    auto dataSize = libopendglab->kotlin.root.getByteArraySize(dataBytes);
    auto data = QByteArray(dataSize, Qt::Initialization::Uninitialized);
    libopendglab->kotlin.root.toNativeByteArray(data.data(), dataBytes);
    eStimService->writeCharacteristic(abpower, data, QLowEnergyService::WriteWithoutResponse);
}
void DGLabDevice::deviceBatteryServiceStateChanged(QLowEnergyService::ServiceState state){
    if (!batteryService) return;
    if (state == QLowEnergyService::ServiceDiscovered) {
        auto uuid = QBluetoothUuid(
                QUuid(
                        Global::DGLabServices::DeviceStatus::Characteristic::electric
                )
        );
        const QLowEnergyCharacteristic battery = batteryService->characteristic(uuid);
        if (!battery.isValid()){
            qDebug() << "Battery Not Found";
            return;
        }
        const QLowEnergyDescriptor m_notificationDescBattery = battery.descriptor(
                QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDescBattery.isValid()) {
            // enable notification
            batteryService->writeDescriptor(m_notificationDescBattery, QByteArray::fromHex("0100"));
        }
        batteryService->readCharacteristic(battery);
    }
}
void DGLabDevice::deviceEStimServiceStateChanged(QLowEnergyService::ServiceState state){
    if (!eStimService) return;
    if (state == QLowEnergyService::ServiceDiscovered) {
        auto uuidSetup = QBluetoothUuid(
                QUuid(
                        Global::DGLabServices::EStimStatus::Characteristic::setup
                )
        );
        const QLowEnergyCharacteristic setup = eStimService->characteristic(uuidSetup);
        if (!setup.isValid()){
            qDebug() << "Setup Not Found";
            return;
        }
        eStimService->readCharacteristic(setup);

        auto uuidABPower = QBluetoothUuid(
                QUuid(
                        Global::DGLabServices::EStimStatus::Characteristic::abpower
                )
        );
        const QLowEnergyCharacteristic abpower = eStimService->characteristic(uuidABPower);
        if (!abpower.isValid()){
            qDebug() << "ABPower Not Found";
            return;
        }
        const QLowEnergyDescriptor abPowerDescriptor = abpower.descriptor(
                QBluetoothUuid::ClientCharacteristicConfiguration);
        if (abPowerDescriptor.isValid()) {
            eStimService->writeDescriptor(abPowerDescriptor, QByteArray::fromHex("0100"));
        }
    }
}
void DGLabDevice::deviceBatteryCharacteristicArrived(const QLowEnergyCharacteristic&, const QByteArray & array) {
    libopendglab_ExportedSymbols* libopendglab = libopendglab_symbols();
    auto batteryValue = array.data();
    auto deviceState = libopendglab->kotlin.root.OpenDGLab.get_deviceStatus(openDgLab);
    auto electric = libopendglab->kotlin.root.OpenDGLab.DeviceStatus.get_electric(deviceState);
    auto barray = libopendglab->kotlin.root.createByteArray((void *)batteryValue, array.size());
    auto level = libopendglab->kotlin.root.OpenDGLab.DeviceStatus.Electric.onChange(electric, barray);
    uiDeviceItem->changeBattery(level);
    uiDeviceItem->update();
}
void DGLabDevice::deviceEStimCharacteristicArrived(const QLowEnergyCharacteristic& characteristic, const QByteArray & array) {
    libopendglab_ExportedSymbols* libopendglab = libopendglab_symbols();
    if (characteristic.uuid() == QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::setup))) {
        auto setupValue = array.data();
        auto eStimState = libopendglab->kotlin.root.OpenDGLab.get_eStimStatus(openDgLab);
        auto setup = libopendglab->kotlin.root.OpenDGLab.EStimStatus.get_setup(eStimState);
        auto barray = libopendglab->kotlin.root.createByteArray((void *)setupValue, array.size());
        libopendglab->kotlin.root.OpenDGLab.EStimStatus.Setup.read(setup, barray);
        uiDeviceOperator->setEnabled(true);
        uiDeviceOperator->update();
        uiDeviceItem->changeState(DeviceStateEnum::DeviceState::READY);
        uiDeviceItem->update();
        auto wave = libopendglab->kotlin.root.OpenDGLab.EStimStatus.get_wave(eStimState);
        auto waveCenterA = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.getWaveCenterA(wave);
        auto waveCenterB = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.getWaveCenterB(wave);
        auto sWaveA = uiDeviceOperator->getWaveA();
        auto sWaveB = uiDeviceOperator->getWaveB();
        auto cWave = Global::basicWaveNameList.contains(sWaveA);
        if (cWave) {
            auto waveA = libopendglab->kotlin.root.WaveCenter.Companion.getBasicWave(libopendglab->kotlin.root.WaveCenter.Companion._instance(), sWaveA.toUtf8().data());
            auto convertWaveA = libopendglab->kotlin.root.convertBasicWaveDataToBasicWave(waveA);
            libopendglab->kotlin.root.WaveCenter.selectWave(waveCenterA, convertWaveA);
        } else {
            cWave = Global::touchWaveNameList.contains(sWaveA);
            if (cWave) {
                auto waveA = libopendglab->kotlin.root.WaveCenter.Companion.getTouchWave(libopendglab->kotlin.root.WaveCenter.Companion._instance(), sWaveA.toUtf8().data());
                auto convertWaveA = libopendglab->kotlin.root.convertTouchWaveDataToBasicWave(waveA);
                libopendglab->kotlin.root.WaveCenter.selectWave(waveCenterA, convertWaveA);
            }
        }
        cWave = Global::basicWaveNameList.contains(sWaveB);
        if (cWave) {
            auto waveB = libopendglab->kotlin.root.WaveCenter.Companion.getBasicWave(libopendglab->kotlin.root.WaveCenter.Companion._instance(), sWaveB.toUtf8().data());
            auto convertWaveB = libopendglab->kotlin.root.convertBasicWaveDataToBasicWave(waveB);
            libopendglab->kotlin.root.WaveCenter.selectWave(waveCenterB, convertWaveB);
        } else {
            cWave = Global::touchWaveNameList.contains(sWaveA);
            if (cWave) {
                auto waveB = libopendglab->kotlin.root.WaveCenter.Companion.getTouchWave(libopendglab->kotlin.root.WaveCenter.Companion._instance(), sWaveB.toUtf8().data());
                auto convertWaveB = libopendglab->kotlin.root.convertTouchWaveDataToBasicWave(waveB);
                libopendglab->kotlin.root.WaveCenter.selectWave(waveCenterB, convertWaveB);
            }
        }
        waveSenderTimerThread->start();
    } else if (characteristic.uuid() == QBluetoothUuid(QUuid(Global::DGLabServices::EStimStatus::Characteristic::abpower))) {
        auto value = array.data();
        auto eStimState = libopendglab->kotlin.root.OpenDGLab.get_eStimStatus(openDgLab);
        auto abpower = libopendglab->kotlin.root.OpenDGLab.EStimStatus.get_abPower(eStimState);
        auto barray = libopendglab->kotlin.root.createByteArray((void *)value, array.size());
        libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.onChange(abpower, barray);
        auto aPower = libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.getAPower(abpower);
        auto bPower = libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.getBPower(abpower);
        powerA = aPower;
        powerB = bPower;
        emit powerUpdate(DeviceStateEnum::DeviceChannel::CHANNEL_A, powerA);
        emit powerUpdate(DeviceStateEnum::DeviceChannel::CHANNEL_B, powerB);
        if (remoteLocked) {
            // 通知锁定客户端设备更新
            for (auto r: Global::remoteList) {
                r->sendPowerUpdate(getID(), powerA, powerB);
            }
        }
    }
}


void DGLabDevice::clearCustomWaveAPI(DeviceStateEnum::DeviceChannel channel) {
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
    {
        QMutexLocker locker(&customWaveAMutex);
        customWaveA.empty();
    }
        break;
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
    {
        QMutexLocker locker(&customWaveBMutex);
        customWaveB.empty();
    }
        break;
    }
}
void DGLabDevice::addCustomWaveAPI(DeviceStateEnum::DeviceChannel channel, QByteArray qbytes) {
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
    {
        QMutexLocker locker(&customWaveAMutex);
        customWaveA.push_back(qbytes);
    }
        break;
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
    {
        QMutexLocker locker(&customWaveBMutex);
        customWaveB.push_back(qbytes);
    }
        break;
    }
}
QString DGLabDevice::getWaveAPI(DeviceStateEnum::DeviceChannel channel) {
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
        return uiDeviceOperator->getWaveA();
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
        return uiDeviceOperator->getWaveB();
    }
    return false;
}
void DGLabDevice::setWaveAPI(DeviceStateEnum::DeviceChannel channel, QString str) {
    QString wave;
    if (Global::basicWaveNameList.indexOf(str) >= 0 || Global::touchWaveNameList.indexOf(str) >= 0) {
        wave = str;
    } else {
        wave = "外部输入";
    }
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
    {
        uiDeviceOperator->setWaveA(wave);
    }
        break;
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
    {

        uiDeviceOperator->setWaveB(wave);
    }
        break;
    }
}
int DGLabDevice::getStrengthAPI(DeviceStateEnum::DeviceChannel channel) {
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
        return powerA;
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
        return powerB;
    }
    return false;
}
void DGLabDevice::setStrengthAPI(int level_A, int level_B) {
    changePower(level_A, level_B);
}
void DGLabDevice::setDeviceRemoteLocked(bool lock) {
    remoteLocked = lock;
    if (remoteLocked) {
        uiDeviceOperator->setAutoChange(DeviceStateEnum::DeviceChannel::CHANNEL_A, false);
        uiDeviceOperator->setAutoChange(DeviceStateEnum::DeviceChannel::CHANNEL_B, false);
        uiDeviceItem->changeState(DeviceStateEnum::DeviceState::READY_REMOTEMANAGED);
        uiDeviceOperator->setEnabled(false);
    } else {
        if (controller->state() == QLowEnergyController::UnconnectedState) {
            uiDeviceItem->changeState(DeviceStateEnum::DeviceState::UNCONNECTED);
            uiDeviceOperator->setEnabled(false);
        } else {
            uiDeviceItem->changeState(DeviceStateEnum::DeviceState::READY);
            uiDeviceOperator->setEnabled(true);
        }
    }
}
bool DGLabDevice::getDeviceRemoteLocked()
{
    return remoteLocked;
}
