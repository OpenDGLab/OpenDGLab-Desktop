#ifndef REMOTECLIENT_H
#define REMOTECLIENT_H

#include <QObject>
#include <QList>
#include <QTcpSocket>
#include <QWebSocket>

class RemoteClient : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClient(QTcpSocket *tcpSocket, QObject *parent = nullptr);
    explicit RemoteClient(QWebSocket *webSocket, QObject *parent = nullptr);
    ~RemoteClient() override ;
    void sendConnected(const QString& _uuid, const QString& token, bool isAuth);
    void sendDeviceReset(const QString& deviceId);
    void sendPowerUpdate(const QString &deviceId, int channel_a, int channel_b);
    void close();
    QString getUuid();

private:
    bool isWS = false;
    QTcpSocket *tcpSocket = nullptr;
    QWebSocket *wsSocket = nullptr;
    void readyRead(QByteArray data);
    bool isAuthed = false;
    QString uuid;
    QList<QString> locked;

signals:
    void doAuth(RemoteClient *client, QString appName, QString appId, QString token);
};

#endif // REMOTECLIENT_H
