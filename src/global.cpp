#include "global.h"
#include <libopendglab_api.h>

namespace Global {
    QList<QString> basicWaveNameList;
    QList<QString> touchWaveNameList;
    QString deviceName;
    namespace DGLabServices {
        namespace DeviceStatus {
            QString service;
            namespace Characteristic {
                QString electric;
            }
        }
        namespace EStimStatus {
            QString service;
            namespace Characteristic {
                QString setup;
                QString abpower;
                QString waveA;
                QString waveB;
            }
        }
    }
    bool skipError = false;
    bool skipWarning = false;
    Remote *remote = new Remote();
    QMutex mutex;
    QList<DGLabDevice*> dglabList;
    QList<RemoteClient*> remoteList;
    QBluetoothDeviceDiscoveryAgent *bleScanAgent;
    void initGlobal() {
        bleScanAgent = new QBluetoothDeviceDiscoveryAgent();
        bleScanAgent->setLowEnergyDiscoveryTimeout(5000);
        libopendglab_ExportedSymbols* libopendglab = libopendglab_symbols();
        auto basicWave = libopendglab->kotlin.root.WaveCenter.Companion.getBasicWaveList(libopendglab->kotlin.root.WaveCenter.Companion._instance());
        auto basicWaveSize = libopendglab->kotlin.root.getStringArraySize(basicWave);
        for(int i = 0; i < basicWaveSize; i++) {
            auto item = libopendglab->kotlin.root.getStringArrayItemAt(basicWave, i);
            basicWaveNameList.append(QString::fromUtf8(item));
            libopendglab->DisposeString(item);
        }
        auto touchWave = libopendglab->kotlin.root.WaveCenter.Companion.getTouchWaveList(libopendglab->kotlin.root.WaveCenter.Companion._instance());
        auto touchWaveSize = libopendglab->kotlin.root.getStringArraySize(touchWave);
        for(int i = 0; i < touchWaveSize; i++) {
            auto item = libopendglab->kotlin.root.getStringArrayItemAt(touchWave, i);
            touchWaveNameList.append(QString::fromUtf8(item));
            libopendglab->DisposeString(item);
        }
        auto dlabName = libopendglab->kotlin.root.OpenDGLab.Device.Companion.getName(libopendglab->kotlin.root.OpenDGLab.Device.Companion._instance());
        deviceName = QString::fromUtf8(dlabName);
        libopendglab->DisposeString(dlabName);
        //DeviceStatus
        //Service
        auto batteryServiceUuid = libopendglab->kotlin.root.OpenDGLab.DeviceStatus.Companion.getUUID(libopendglab->kotlin.root.OpenDGLab.DeviceStatus.Companion._instance());
        Global::DGLabServices::DeviceStatus::service = QString::fromUtf8(batteryServiceUuid);
        libopendglab->DisposeString(batteryServiceUuid);
        //Characterisitc
        auto batteryCharacteristic = libopendglab->kotlin.root.OpenDGLab.DeviceStatus.Electric.Companion.getUUID(
                libopendglab->kotlin.root.OpenDGLab.DeviceStatus.Electric.Companion._instance()
        );
        Global::DGLabServices::DeviceStatus::Characteristic::electric = QString::fromUtf8(batteryCharacteristic);
        libopendglab->DisposeString(batteryCharacteristic);
        //EStimStatus
        //Service
        auto estimServiceUuid = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Companion.getUUID(libopendglab->kotlin.root.OpenDGLab.EStimStatus.Companion._instance());
        Global::DGLabServices::EStimStatus::service = QString::fromUtf8(estimServiceUuid);
        libopendglab->DisposeString(estimServiceUuid);
        //Characterisitc
        auto setupCharacterisitc = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Setup.Companion.getUUID(
                libopendglab->kotlin.root.OpenDGLab.EStimStatus.Setup.Companion._instance()
        );
        Global::DGLabServices::EStimStatus::Characteristic::setup = QString::fromUtf8(setupCharacterisitc);
        libopendglab->DisposeString(setupCharacterisitc);
        auto abpowerCharacterisitc = libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.Companion.getUUID(
                libopendglab->kotlin.root.OpenDGLab.EStimStatus.ABPower.Companion._instance()
        );
        Global::DGLabServices::EStimStatus::Characteristic::abpower = QString::fromUtf8(abpowerCharacterisitc);
        libopendglab->DisposeString(abpowerCharacterisitc);
        auto waveACharacterisitc = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.Companion.getUUIDA(
                libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.Companion._instance()
        );
        Global::DGLabServices::EStimStatus::Characteristic::waveA = QString::fromUtf8(waveACharacterisitc);
        libopendglab->DisposeString(waveACharacterisitc);
        auto waveBCharacterisitc = libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.Companion.getUUIDB(
                libopendglab->kotlin.root.OpenDGLab.EStimStatus.Wave.Companion._instance()
        );
        Global::DGLabServices::EStimStatus::Characteristic::waveB = QString::fromUtf8(waveBCharacterisitc);
        libopendglab->DisposeString(waveBCharacterisitc);

    }

    QList<QString> getWaveList()
    {
        QList<QString> list;
        list.append(basicWaveNameList);
        list.append(touchWaveNameList);
        return list;
    }

}
