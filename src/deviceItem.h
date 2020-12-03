#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include <QWidget>
#include "deviceStateEnum.h"

namespace Ui {
class DeviceItem;
}

class DeviceItem : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceItem(QString id, QString address, QString name, QWidget *parent = nullptr);
    ~DeviceItem() override;
    void changeState(DeviceStateEnum::DeviceState state);
    void changeBattery(int level);
    DeviceStateEnum::DeviceState getState();

private:
    Ui::DeviceItem *ui;
    DeviceStateEnum::DeviceState deviceState = DeviceStateEnum::DeviceState::UNCONNECTED;
};

#endif // DEVICEITEM_H
