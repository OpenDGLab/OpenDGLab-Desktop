#include "remoteClient.h"
#include "global.h"
#include <proto/app.pb.h>
#include <QDebug>
RemoteClient::RemoteClient(QTcpSocket *tcpSocket, QObject *parent) : QObject(parent), tcpSocket(tcpSocket), isWS(false)
{
    connect(tcpSocket, &QTcpSocket::disconnected, this, [this](){
        for (auto r: Global::dglabList) {
            if (locked.contains(r->getID())) {
                r->setDeviceRemoteLocked(false);
                r->clearCustomWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_A);
                r->clearCustomWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_B);
            }
        }
        delete this;
    });
    connect(tcpSocket, &QTcpSocket::readyRead, this, [this](){
        auto bytes = this->tcpSocket->readAll();
        readyRead(bytes);
    });
}
#ifndef __linux__
RemoteClient::RemoteClient(QWebSocket *wsSocket, QObject *parent) : QObject(parent), wsSocket(wsSocket), isWS(true)
{
    connect(wsSocket, &QWebSocket::disconnected, this, [this](){
        delete this;
    });
    connect(wsSocket, &QWebSocket::binaryMessageReceived, this, [this](const QByteArray &message){
        readyRead(message);
    });
}
#endif

RemoteClient::~RemoteClient() {
    isAuthed = false;
    if (isWS) {
#ifndef __linux__
        wsSocket->close();
#endif
    } else {
        tcpSocket->close();
    }
    Global::remoteList.removeOne(this);
}

void RemoteClient::readyRead(QByteArray data) {
    auto dgRequest = com::github::opendglab::DGRequest();
    dgRequest.ParseFromArray(data.data(), data.size());
    auto response = com::github::opendglab::DGResponse();
    response.set_version(1);
    if (dgRequest.event() == com::github::opendglab::DGEvent::PING) {
        return;
    }
    if (!isAuthed && dgRequest.event() != com::github::opendglab::DGEvent::CONNECT) {
        response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
        response.set_error(com::github::opendglab::DGError::UNAUTHED);
    } else {
        switch (dgRequest.event()) {
            case com::github::opendglab::DGEvent::CONNECT:
            {
                if (!isAuthed) {
                    const auto& conn = dgRequest.connect();
                    emit doAuth(this, QString::fromStdString(conn.appname()), QString::fromStdString(conn.uuid()), QString::fromStdString(conn.token()));
                }
                return;
            }
            case com::github::opendglab::GETDEVICE:
            {
                response.set_event(com::github::opendglab::DGEvent::GETDEVICE);
                auto deviceList = new com::github::opendglab::DGResponse_DGDeviceList();
                for (auto r: Global::dglabList) {
                    if(r->getState() == DeviceStateEnum::DeviceState::READY || r->getState() == DeviceStateEnum::DeviceState::READY_REMOTEMANAGED) {
                        auto device = deviceList->add_devices();
                        device->set_id(r->getID().toStdString());
                        bool isLocked = r->getState() == DeviceStateEnum::DeviceState::READY_REMOTEMANAGED;
                        device->set_islockedbyremote(isLocked);
                        bool isLockedByMe = this->locked.contains(r->getID());
                        device->set_islockedbyme(isLockedByMe);
                    }
                }
                response.set_allocated_devicelist(deviceList);
                break;
            }
            case com::github::opendglab::LOCKDEVICE:
            {
                const auto& device = dgRequest.device();
                auto deviceId = QString::fromStdString(device.deviceid());
                if (deviceId.isEmpty()) {
                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                    response.set_error(com::github::opendglab::DGError::UNKNOWN);
                } else {
                    for (auto r: Global::dglabList) {
                        if (r->getID() == deviceId) {
                            switch (r->getState()) {
                                case DeviceStateEnum::DeviceState::UNCONNECTED:
                                case DeviceStateEnum::CONNECTED:
                                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                                    response.set_error(com::github::opendglab::DGError::DEVICEOFFLINE);
                                    break;
                                case DeviceStateEnum::READY:
                                {
                                    r->setDeviceRemoteLocked(true);
                                    locked.append(deviceId);
                                    response.set_event(com::github::opendglab::DGEvent::LOCKDEVICE);
                                    auto mDevice = new com::github::opendglab::DGResponse_DGDevice();
                                    mDevice->set_id(deviceId.toStdString());
                                    mDevice->set_islockedbyme(true);
                                    mDevice->set_islockedbyremote(true);
                                    response.set_allocated_device(mDevice);
                                    break;
                                }
                                case DeviceStateEnum::READY_REMOTEMANAGED:
                                    if (locked.contains(deviceId)) {
                                        response.set_event(com::github::opendglab::DGEvent::LOCKDEVICE);
                                        auto mDevice = new com::github::opendglab::DGResponse_DGDevice();
                                        mDevice->set_id(deviceId.toStdString());
                                        mDevice->set_islockedbyme(true);
                                        mDevice->set_islockedbyremote(true);
                                        response.set_allocated_device(mDevice);
                                    } else {
                                        response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                                        response.set_error(com::github::opendglab::DGError::DEVICENOTLOCKBYYOU);
                                    }
                                    break;
                            }
                            goto escape_route_lock;
                        }
                    }
                    escape_route_lock:
                    break;
                }
            }
            case com::github::opendglab::UNLOCKDEVICE:
            {
                const auto& device = dgRequest.device();
                auto deviceId = QString::fromStdString(device.deviceid());
                if (deviceId.isEmpty()) {
                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                    response.set_error(com::github::opendglab::DGError::UNKNOWN);
                } else {
                    for (auto r: Global::dglabList) {
                        if (r->getID() == deviceId) {
                            switch (r->getState()) {
                                case DeviceStateEnum::DeviceState::UNCONNECTED:
                                case DeviceStateEnum::CONNECTED:
                                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                                    response.set_error(com::github::opendglab::DGError::DEVICEOFFLINE);
                                    break;
                                case DeviceStateEnum::READY:
                                {
                                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                                    response.set_error(com::github::opendglab::DGError::DEVICENOTLOCK);
                                    break;
                                }
                                case DeviceStateEnum::READY_REMOTEMANAGED:
                                    if (locked.contains(deviceId)) {
                                        r->setDeviceRemoteLocked(false);
                                        r->clearCustomWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_A);
                                        r->clearCustomWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_B);
                                        response.set_event(com::github::opendglab::DGEvent::UNLOCKDEVICE);
                                        auto mDevice = new com::github::opendglab::DGResponse_DGDevice();
                                        mDevice->set_id(deviceId.toStdString());
                                        mDevice->set_islockedbyme(false);
                                        mDevice->set_islockedbyremote(false);
                                        response.set_allocated_device(mDevice);
                                        locked.removeOne(deviceId);
                                    } else {
                                        response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                                        response.set_error(com::github::opendglab::DGError::DEVICENOTLOCKBYYOU);
                                    }
                                    break;
                            }
                            goto escape_route_unlock;
                        }
                    }
                    escape_route_unlock:
                    break;
                }
            }
            case com::github::opendglab::GETSTRENGTH:
            {
                const auto& device = dgRequest.device();
                auto deviceId = QString::fromStdString(device.deviceid());
                if (locked.contains(deviceId)) {
                    for (auto r: Global::dglabList) {
                        if (r->getID() == deviceId) {
                            auto strength = new com::github::opendglab::DGResponse_DGDeviceStrength();
                            response.set_event(com::github::opendglab::DGEvent::GETSTRENGTH);
                            strength->set_strengtha(r->getStrengthAPI(DeviceStateEnum::DeviceChannel::CHANNEL_A));
                            strength->set_strengthb(r->getStrengthAPI(DeviceStateEnum::DeviceChannel::CHANNEL_B));
                            response.set_allocated_strength(strength);
                            break;
                        }
                    }
                } else {
                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                    response.set_error(com::github::opendglab::DGError::DEVICENOTLOCKBYYOU);
                }
                break;
            }
            case com::github::opendglab::SETSTRENGTH:
            {
                const auto& device = dgRequest.device();
                auto deviceId = QString::fromStdString(device.deviceid());
                const auto& strength = dgRequest.strength();
                if (locked.contains(deviceId)) {
                    for (auto r: Global::dglabList) {
                        if (r->getID() == deviceId) {
                            r->setStrengthAPI(strength.strengtha(), strength.strengthb());
                            return;
                        }
                    }
                } else {
                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                    response.set_error(com::github::opendglab::DGError::DEVICENOTLOCKBYYOU);
                }
                return;
            }
            case com::github::opendglab::GETWAVELIST:
            {
                response.set_event(com::github::opendglab::DGEvent::GETWAVELIST);
                auto* waveList = new com::github::opendglab::DGResponse_DGWaveList();
                for (const auto& w: Global::basicWaveNameList) {
                    waveList->add_wave(w.toStdString());
                }
                for (const auto& w: Global::touchWaveNameList) {
                    waveList->add_wave(w.toStdString());
                }
                response.set_allocated_wavelist(waveList);
                break;
            }
            case com::github::opendglab::GETWAVE:
            {
                const auto& device = dgRequest.device();
                auto deviceId = QString::fromStdString(device.deviceid());
                const auto& strength = dgRequest.strength();
                if (locked.contains(deviceId)) {
                    for (auto r: Global::dglabList) {
                        if (r->getID() == deviceId) {
                            auto oDevice = new com::github::opendglab::DGResponse_DGDeviceID();
                            auto wave = new com::github::opendglab::DGResponse_DGWave();
                            response.set_event(com::github::opendglab::DGEvent::GETWAVE);
                            switch (device.devicechannel()) {
                                case com::github::opendglab::CHANNEL_A:
                                {
                                    auto w = r->getWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_A);
                                    wave->set_wave(w.toStdString());
                                    oDevice->set_deviceid(deviceId.toStdString());
                                    oDevice->set_devicechannel(com::github::opendglab::CHANNEL_A);
                                    break;
                                }
                                case com::github::opendglab::CHANNEL_B:
                                {
                                    auto w = r->getWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_B);
                                    wave->set_wave(w.toStdString());
                                    oDevice->set_deviceid(deviceId.toStdString());
                                    oDevice->set_devicechannel(com::github::opendglab::CHANNEL_B);
                                    break;
                                }
                                default:
                                    break;
                            }
                            break;
                        }
                    }
                } else {
                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                    response.set_error(com::github::opendglab::DGError::DEVICENOTLOCKBYYOU);
                }
                break;
            }
            case com::github::opendglab::SETWAVE:
            {
                const auto& device = dgRequest.device();
                auto deviceId = QString::fromStdString(device.deviceid());
                const auto& strength = dgRequest.strength();
                auto waveName = QString::fromStdString(dgRequest.wave().wavename());
                if (locked.contains(deviceId)) {
                    for (auto r: Global::dglabList) {
                        if (r->getID() == deviceId) {
                            response.set_event(com::github::opendglab::DGEvent::SETWAVE);
                            auto oDevice = new com::github::opendglab::DGResponse_DGDeviceID();
                            auto wave = new com::github::opendglab::DGResponse_DGWave();
                            switch (device.devicechannel()) {
                                case com::github::opendglab::CHANNEL_A:
                                {
                                    r->setWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_A, waveName);
                                    auto w = r->getWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_A);
                                    wave->set_wave(w.toStdString());
                                    oDevice->set_deviceid(deviceId.toStdString());
                                    oDevice->set_devicechannel(com::github::opendglab::CHANNEL_A);
                                    response.set_allocated_deviceid(oDevice);
                                    response.set_allocated_wavename(wave);
                                    break;
                                }
                                case com::github::opendglab::CHANNEL_B:
                                {
                                    r->setWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_B, waveName);
                                    auto w = r->getWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_B);
                                    wave->set_wave(w.toStdString());
                                    oDevice->set_deviceid(deviceId.toStdString());
                                    oDevice->set_devicechannel(com::github::opendglab::CHANNEL_B);
                                    response.set_allocated_deviceid(oDevice);
                                    response.set_allocated_wavename(wave);
                                    break;
                                }
                                default:
                                    break;
                            }
                            break;
                        }
                    }
                } else {
                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                    response.set_error(com::github::opendglab::DGError::DEVICENOTLOCKBYYOU);
                }
                break;
            }
            case com::github::opendglab::CUSTOMWAVE:
            {
                const auto& device = dgRequest.device();
                auto deviceId = QString::fromStdString(device.deviceid());
                const auto& customWave = dgRequest.customwave();
                if (locked.contains(deviceId)) {
                    for (auto r: Global::dglabList) {
                        if (r->getID() == deviceId) {
                            switch (device.devicechannel()) {
                                case com::github::opendglab::CHANNEL_A:
                                {
                                    for (const auto& c: customWave) {
                                        if(c.bytes().size() == 3) {
                                            auto array = new QByteArray(3, Qt::Initialization::Uninitialized);
                                            const auto& cb = c.bytes();
                                            array->data()[0] = cb[0];
                                            array->data()[1] = cb[1];
                                            array->data()[2] = cb[2];
                                            r->addCustomWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_A, *array);
                                        }
                                    }
                                    break;
                                }
                                case com::github::opendglab::CHANNEL_B:
                                {
                                    for (const auto& c: customWave) {
                                        if(c.bytes().size() == 3) {
                                            auto array = new QByteArray(3, Qt::Initialization::Uninitialized);
                                            const auto& cb = c.bytes();
                                            array->data()[0] = cb[0];
                                            array->data()[1] = cb[1];
                                            array->data()[2] = cb[2];
                                            r->addCustomWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_B, *array);
                                        }
                                    }
                                    break;
                                }
                                default:
                                    break;
                            }
                            return;
                        }
                    }
                } else {
                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                    response.set_error(com::github::opendglab::DGError::DEVICENOTLOCKBYYOU);
                }
                break;
            }
            case com::github::opendglab::CLEARCUSTOM:
            {
                const auto& device = dgRequest.device();
                auto deviceId = QString::fromStdString(device.deviceid());
                if (locked.contains(deviceId)) {
                    for (auto r: Global::dglabList) {
                        if (r->getID() == deviceId) {
                            switch (device.devicechannel()) {
                                case com::github::opendglab::CHANNEL_A:
                                    r->clearCustomWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_A);
                                    break;
                                case com::github::opendglab::CHANNEL_B:
                                    r->clearCustomWaveAPI(DeviceStateEnum::DeviceChannel::CHANNEL_B);
                                    break;
                                default:
                                    break;
                            }
                            return;
                        }
                    }
                } else {
                    response.set_event(com::github::opendglab::DGEvent::CANTDOTHIS);
                    response.set_error(com::github::opendglab::DGError::DEVICENOTLOCKBYYOU);
                }
                break;
            }
            default:
                return;
        }
    }
    QByteArray byteArray(response.SerializeAsString().c_str(), response.ByteSizeLong());
    if(isWS) {
#ifndef __linux__
        wsSocket->sendBinaryMessage(byteArray);
#endif
    } else {
        tcpSocket->write(byteArray);
    }
}

void RemoteClient::sendConnected(const QString& _uuid, const QString& token, bool isAuth) {
    auto response = com::github::opendglab::DGResponse();
    response.set_version(1);
    response.set_event(com::github::opendglab::DGEvent::CONNECT);
    auto *conn = new com::github::opendglab::DGResponse_DGConnect();
    conn->set_token(token.toStdString());
    response.set_allocated_connect(conn);
    QByteArray byteArray(response.SerializeAsString().c_str(), response.ByteSizeLong());
    this->isAuthed = isAuth;
    this->uuid = _uuid;
    if(isWS) {
#ifndef __linux__
        wsSocket->sendBinaryMessage(byteArray);
#endif
    } else {
        tcpSocket->write(byteArray);
    }
    if (!isAuth) {
        close();
    }
}

void RemoteClient::close() {
    if (isWS) {
#ifndef __linux__
        wsSocket->close();
#endif
    } else {
        tcpSocket->close();
    }
}

QString RemoteClient::getUuid() {
    return uuid;
}

void RemoteClient::sendDeviceReset(const QString &deviceId) {
    if (locked.contains(deviceId)) {
        auto response = com::github::opendglab::DGResponse();
        response.set_version(1);
        response.set_event(com::github::opendglab::DGEvent::DEVICERESET);
        auto sDeviceId = new com::github::opendglab::DGResponse_DGDeviceID();
        sDeviceId->set_deviceid(deviceId.toStdString());
        response.set_allocated_deviceid(sDeviceId);
        QByteArray byteArray(response.SerializeAsString().c_str(), response.ByteSizeLong());
        if(isWS) {
#ifndef __linux__
            wsSocket->sendBinaryMessage(byteArray);
#endif
        } else {
            tcpSocket->write(byteArray);
        }
        locked.removeOne(deviceId);
    }
}

void RemoteClient::sendPowerUpdate(const QString &deviceId, int channel_a, int channel_b) {
    if (locked.contains(deviceId)) {
        auto response = com::github::opendglab::DGResponse();
        response.set_version(1);
        response.set_event(com::github::opendglab::DGEvent::GETSTRENGTH);
        auto sDeviceId = new com::github::opendglab::DGResponse_DGDeviceID();
        sDeviceId->set_deviceid(deviceId.toStdString());
        response.set_allocated_deviceid(sDeviceId);
        auto strength = new com::github::opendglab::DGResponse_DGDeviceStrength();
        strength->set_strengtha(channel_a);
        strength->set_strengthb(channel_b);
        response.set_allocated_strength(strength);
        QByteArray byteArray(response.SerializeAsString().c_str(), response.ByteSizeLong());
        if(isWS) {
#ifndef __linux__
            wsSocket->sendBinaryMessage(byteArray);
#endif
        } else {
            tcpSocket->write(byteArray);
        }
    }
}

void RemoteClient::sendPing() {
    if (isAuthed) {
        auto response = com::github::opendglab::DGResponse();
        response.set_version(1);
        response.set_event(com::github::opendglab::DGEvent::PING);
        QByteArray byteArray(response.SerializeAsString().c_str(), response.ByteSizeLong());
        if(isWS) {
#ifndef __linux__
            wsSocket->sendBinaryMessage(byteArray);
#endif
        } else {
            tcpSocket->write(byteArray);
        }
    }
}
