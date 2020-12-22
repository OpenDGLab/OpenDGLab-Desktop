#ifndef GLOBAL_H
#define GLOBAL_H
#define SKIPNEXTERROR(N) \
    ( \
      (Global::skipError = true),\
      (N), \
      (Global::skipError = false),\
      (void)0 \
    )
#define SKIPNEXTQWARNING(N) \
    ( \
      (Global::skipWarning = true),\
      (N), \
      (Global::skipWarning = false),\
      (void)0 \
    )
#include <QList>
#include <QMutex>
#include <QBluetoothDeviceDiscoveryAgent>
#include "dglabDevice.h"
#include "remote.h"

namespace Global {
    extern QList<QString> basicWaveNameList;
    extern QList<QString> touchWaveNameList;
    extern QString deviceName;
    namespace DGLabServices {
        namespace DeviceStatus {
            extern QString service;
            namespace Characteristic {
                extern QString electric;
            }
        }
        namespace EStimStatus {
            extern QString service;
            namespace Characteristic {
                extern QString setup;
                extern QString abpower;
                extern QString waveA;
                extern QString waveB;
            }
        }
    }
    extern bool skipError;
    extern bool skipWarning;
    extern Remote* remote;
    extern QList<RemoteClient*> remoteList;
    extern QList<DGLabDevice*> dglabList;
    extern QBluetoothDeviceDiscoveryAgent *bleScanAgent;
    extern QMutex mutex;
    //extern Remote *remote;
    void initGlobal();
    QList<QString> getWaveList();
}

#endif // GLOBAL_H
