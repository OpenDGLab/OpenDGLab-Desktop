#ifndef DEVICEOPERATOR_H
#define DEVICEOPERATOR_H

#include "deviceStateEnum.h"
#include <QWidget>
#include <QRandomGenerator>
namespace Ui {
class DeviceOperator;
}

class DeviceOperator : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceOperator(QString id, QWidget *parent = nullptr);
    ~DeviceOperator() override;
    QString getWaveA();
    QString getWaveB();
    void setWaveA(QString);
    void setWaveB(QString);
    void setAutoChange(DeviceStateEnum::DeviceChannel, bool);
    bool getAutoChange(DeviceStateEnum::DeviceChannel);
    void checkIfNeedChangeWave();

public slots:
    void setPower(DeviceStateEnum::DeviceChannel channel, int level);

private slots:


private:
    Ui::DeviceOperator *ui;
    int autoATimer = 0;
    int autoBTimer = 0;
    QRandomGenerator random;

signals:
    void startBoost(DeviceStateEnum::DeviceChannel, int);
    void stopBoost(DeviceStateEnum::DeviceChannel, int);
    void changePower(int, int);
    void changeWave(DeviceStateEnum::DeviceChannel, QString);
};

#endif // DEVICEOPERATOR_H
