#ifndef MAINWINDOWCONTROLLER_H
#define MAINWINDOWCONTROLLER_H
#include <QObject>
#include "bleConnectThread.h"
#include "autoWaveChangerThread.h"

class MainWindowController : public QObject
{
    Q_OBJECT
public:
    explicit MainWindowController(QObject *parent = nullptr);
    ~MainWindowController() override;
    void startAllBoost(int);
    void stopAllBoost();
    void shutdown();
private:
    BLEConnectThread connectThread;
    AutoWaveChangerThread autoWaveChangerThread = AutoWaveChangerThread(this);
signals:

};

#endif // MAINWINDOWCONTROLLER_H
