#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QObject>
#include <QList>
#include <QTcpSocket>
#ifndef __linux__
#include <QWebSocket>
#endif

class RemoteClient : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClient(QTcpSocket *tcpSocket, QObject *parent = nullptr);
#ifndef __linux__
    explicit RemoteClient(QWebSocket *webSocket, QObject *parent = nullptr);
#endif
    ~RemoteClient() override ;
    void sendConnected(const QString& _uuid, const QString& token, bool isAuth);
    void sendDeviceReset(const QString& deviceId);
    void sendPowerUpdate(const QString &deviceId, int channel_a, int channel_b);
    void sendPing();
    void close();
    QString getUuid();

private:
    bool isWS = false;
    QTcpSocket *tcpSocket = nullptr;
#ifndef __linux__
    QWebSocket *wsSocket = nullptr;
#endif
    void readyRead(QByteArray data);
    bool isAuthed = false;
    QString uuid;
    QList<QString> locked;

signals:
    void doAuth(RemoteClient *client, QString appName, QString appId, QString token);
};

#endif // REMOTECLIENT_H
