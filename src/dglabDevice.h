#ifndef DGLABDEVICE_H
#define DGLABDEVICE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QThread>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QCryptographicHash>
#include "deviceItem.h"
#include "deviceOperator.h"
#include "waveSenderTimerThread.h"
#include <libopendglab_api.h>
class DGLabDevice : public QObject
{
    Q_OBJECT
public:
    explicit DGLabDevice(const QBluetoothDeviceInfo &_deviceInfo, QObject *parent = nullptr);
    ~DGLabDevice() override;
    QString getDeviceAddressOrUuid();
    QString getID();
    bool isShownOnUi();
    void setShownOnUi();
    DeviceItem* getUiDeviceItem();
    DeviceOperator* getUiDeviceOperator();
    QLowEnergyController* getBLEController();
    void startGlobalBoost(int);
    void stopGlobalBoost();
    DeviceStateEnum::DeviceState getState();

private:
    QBluetoothDeviceInfo deviceInfo;
    QString id;
    QString addressOrUuid;
    QLowEnergyController* controller;
    QLowEnergyService* batteryService;
    QLowEnergyService* eStimService;
    libopendglab_kref_OpenDGLab openDgLab;
    DeviceItem* uiDeviceItem;
    DeviceOperator* uiDeviceOperator;
    bool shownOnUi = false;
    WaveSenderTimerThread* waveSenderTimerThread;
    int powerA = 0;
    int powerB = 0;
    int boostA = 0;
    int boostB = 0;
    int globalBoost = 0;

public slots:
    void waveSender();
    void setWave(DeviceStateEnum::DeviceChannel, QString);
    void startChannelBoost(DeviceStateEnum::DeviceChannel, int boost);
    void stopChannelBoost(DeviceStateEnum::DeviceChannel, int boost);

private slots:
    void deviceConnected();
    void deviceDisconnected();
    void deviceServiceDiscoverFinished();
    void deviceBatteryServiceStateChanged(QLowEnergyService::ServiceState);
    void deviceEStimServiceStateChanged(QLowEnergyService::ServiceState);
    void deviceBatteryCharacteristicArrived(const QLowEnergyCharacteristic&, const QByteArray &);
    void deviceEStimCharacteristicArrived(const QLowEnergyCharacteristic&, const QByteArray &);

signals:
    void powerUpdate(DeviceStateEnum::DeviceChannel, int);
};

#endif // DGLABDEVICE_H
