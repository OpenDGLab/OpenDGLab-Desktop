#ifndef DEVICESTATEENUM_H
#define DEVICESTATEENUM_H

namespace DeviceStateEnum {
    enum DeviceState {
        UNCONNECTED, CONNECTED, READY, READY_REMOTEMANAGED
    };
    enum DeviceChannel {
        CHANNEL_A, CHANNEL_B
    };
}

#endif // DEVICESTATEENUM_H
