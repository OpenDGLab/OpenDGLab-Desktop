#ifndef REMOTE_H
#define REMOTE_H

#include <QObject>
#include <QTcpServer>
#ifndef __linux__
#include <QWebSocketServer>
#endif
#include "remoteClient.h"
class Remote : public QObject
{
    Q_OBJECT
public:
    explicit Remote(QObject *parent = nullptr);
    ~Remote() override;
    bool isStarted();
    bool start();
    void stop();
    void setWSPort(int port);
    void setTcpPort(int port);
    void load();
    void save();
    QMap<QString, QString> preAuth; //UUID, Token
    QMap<QString, QString> preAuthName; //UUID, Token

private:
    int tcpPort;
    int wsPort;
    QList<RemoteClient*> clientList;
    QTcpServer *tcpServer;
#ifndef __linux__
    QWebSocketServer *wsServer;
#endif
    bool isStart = false;
    QString getRandomString();

private slots:
    void tcpNewConnection();
    void wsNewConnection();
    void doAuth(RemoteClient* client, QString appName, QString appId, QString token);

signals:
    void stateChange();

};

#endif // REMOTE_H
