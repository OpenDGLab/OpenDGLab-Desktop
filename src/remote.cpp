#include "remote.h"
#include "global.h"
#include <QMessageBox>
#include <proto/app.pb.h>
#include <QWebSocketCorsAuthenticator>
Remote::Remote(QObject *parent) : QObject(parent), tcpPort(11240), wsPort(11241)
{
    tcpServer = new QTcpServer(this);
    wsServer = new QWebSocketServer("127.0.0.1", QWebSocketServer::SslMode::NonSecureMode, this);
    auto *cors = new QWebSocketCorsAuthenticator("127.0.0.1");
    cors->setAllowed(true);
    wsServer->originAuthenticationRequired(cors);
    connect(wsServer, &QWebSocketServer::originAuthenticationRequired, [](QWebSocketCorsAuthenticator *){
        return true;
    });
    connect(tcpServer, &QTcpServer::newConnection, this, &Remote::tcpNewConnection);
    connect(wsServer, &QWebSocketServer::newConnection, this, &Remote::wsNewConnection);
}

Remote::~Remote()
{
    stop();
}

bool Remote::isStarted() {
    return isStart;
}

bool Remote::start() {
    bool tcpStart = false;
    bool wsStart = false;
    if (!isStart) {
        if(tcpServer->listen(QHostAddress::LocalHost, tcpPort)){
            tcpStart = true;
        }
        if (wsServer->listen(QHostAddress::LocalHost, wsPort)) {
            wsStart = true;
        }
        if (tcpStart && wsStart){
            isStart = true;
            return true;
        }
        if (tcpStart) tcpServer->close();
        if (wsStart) wsServer->close();
        return false;
    }
    return true;
}

void Remote::stop() {
    if (isStart) {
        tcpServer->close();
        wsServer->close();
        for(auto r: Global::remoteList) {
            r->close();
        }
    }
}

void Remote::setWSPort(int port) {
    if (!isStart) wsPort = port;
}

void Remote::setTcpPort(int port) {
    if (!isStart) tcpPort = port;
}

void Remote::tcpNewConnection() {
    auto socket = tcpServer->nextPendingConnection();
    auto* client = new RemoteClient(socket);
    connect(client, &RemoteClient::doAuth, this, &Remote::doAuth);
    Global::remoteList.push_back(client);
}
void Remote::wsNewConnection() {
    auto socket = wsServer->nextPendingConnection();
    auto* client = new RemoteClient(socket);
    connect(client, &RemoteClient::doAuth, this, &Remote::doAuth);
    Global::remoteList.push_back(client);
}

void Remote::doAuth(RemoteClient* client, QString appName, QString appId, QString token) {
    if (appName.isEmpty() || appId.isEmpty()) client->sendConnected("", "", false);
    if (preAuth.contains(appId)) {
        auto pA = preAuth[appId];
        if (pA == token) {
            client->sendConnected(appId, token, true);
        } else {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, "应用连接", QString("%1 正在请求连接 OpenDGLab Desktop 以控制您的设备。").arg(appName), QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                auto nToken = getRandomString();
                client->sendConnected(appId, nToken, true);
                Global::remote->preAuth[appId] = nToken;
                Global::remote->preAuthName[appId] = appName;
            } else {
                client->sendConnected("", "", false);
            }
        }
    } else {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, "应用连接", QString("%1 正在请求连接 OpenDGLab Desktop 以控制您的设备。").arg(appName), QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            auto nToken = getRandomString();
            client->sendConnected(appId, nToken, true);
            Global::remote->preAuth[appId] = nToken;
            Global::remote->preAuthName[appId] = appName;
        } else {
            client->sendConnected("", "", false);
        }
    }
    emit stateChange();
}

QString Remote::getRandomString() {
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = 16;

    QString randomString;
    for(int i=0; i<randomStringLength; ++i)
    {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}

void Remote::load() {

}

void Remote::save() {

}
