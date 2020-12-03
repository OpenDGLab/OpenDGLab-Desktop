#include "deviceItem.h"
#include "ui_deviceItem.h"

DeviceItem::DeviceItem(QString id, QString address, QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceItem)
{
    ui->setupUi(this);
    ui->lbl_ID->setText(id);
    ui->lbl_Mac->setText(address);
    ui->lbl_Name->setText(name);
    ui->lbl_Status->setText("未连接");
}

DeviceItem::~DeviceItem()
{
    delete ui;
}

void DeviceItem::changeBattery(int level)
{
    if (level < 0) {
        ui->pb_battery->setFormat(" 正在配置..");
        ui->pb_battery->setMaximum(0);
        ui->pb_battery->setMinimum(0);
        ui->pb_battery->setValue(0);
    } else {
        ui->pb_battery->setFormat(" 电池：%p%");
        ui->pb_battery->setMaximum(100);
        ui->pb_battery->setMinimum(0);
        ui->pb_battery->setValue(level);
    }
}

DeviceStateEnum::DeviceState DeviceItem::getState()
{
    return deviceState;
}

void DeviceItem::changeState(DeviceStateEnum::DeviceState state)
{
    switch (state) {
    case DeviceStateEnum::DeviceState::CONNECTED:
        ui->lbl_Status->setText("已连接 - 正在配置");
        break;
    case DeviceStateEnum::DeviceState::READY:
        ui->lbl_Status->setText("已连接 - 已就绪");
        break;
    case DeviceStateEnum::DeviceState::UNCONNECTED:
        ui->lbl_Status->setText("未连接");
        break;
    case DeviceStateEnum::DeviceState::READY_REMOTEMANAGED:
        ui->lbl_Status->setText("已连接 - 已就绪（正在被远程管理）");
        break;
    }
    deviceState = state;
}
