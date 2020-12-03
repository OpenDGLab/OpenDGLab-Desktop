#include "scanDevice.h"
#include "ui_scanDevice.h"
#include <QSizePolicy>

ScanDevice::ScanDevice(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ScanDevice),
    mvcController(new ScanDeviceController(this))
{
    ui->setupUi(this);
    this->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    this->setFixedSize(QSize(300, 100));
    connect(mvcController, &ScanDeviceController::eventScanError, this, &ScanDevice::setError);
    connect(mvcController, &ScanDeviceController::eventScanFinished, this, &ScanDevice::setFinished);
    connect(mvcController, &ScanDeviceController::eventScanFound, this, &ScanDevice::setFound);
    connect(ui->btnAddCancel, &QPushButton::clicked, this, &ScanDevice::setCancelled);
}

ScanDevice::~ScanDevice()
{
    delete ui;
}
void ScanDevice::showEvent(QShowEvent*) {
    mvcController->startScan();
}
void ScanDevice::closeEvent(QCloseEvent*) {

}
void ScanDevice::setError() {
    ui->lbl_status->setText("错误：无法扫描");
}
void ScanDevice::setFinished() {
    ui->lbl_status->setText("扫描完成 未发现设备");
}
void ScanDevice::setFound() {
    emit needRefresh();
    this->close();
}
void ScanDevice::setCancelled() {
    this->close();
}
