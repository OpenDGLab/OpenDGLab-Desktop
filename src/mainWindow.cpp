#include <QListWidgetItem>
#include "mainWindow.h"
#include "global.h"
#include "scanDevice.h"
#include "./ui_mainWindow.h"
#include <QMenu>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mvcController(new MainWindowController(this))
{
    ui->setupUi(this);
    connect(ui->btnAddDevice, &QPushButton::clicked, this, &MainWindow::addDevice);
    ui->listDevice->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(ui->listDevice, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(ui->allBoost, &QSlider::valueChanged, this, &MainWindow::allBoostChanged);
    connect(ui->btn_boost, &QPushButton::pressed, this, [this](){
        startAllBoost(ui->allBoost->value());
    });
    connect(ui->btn_boost, &QPushButton::released, this, [this](){
        stopAllBoost();
    });
    loadDevice();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addDevice() {
    auto scanDeviceDialog = new ScanDevice(this);
    connect(scanDeviceDialog, &ScanDevice::needRefresh, this, &MainWindow::refreshDeviceList);
    scanDeviceDialog->show();
}

void MainWindow::showContextMenu(const QPoint &pos) {
    QPoint globalPos = ui->listDevice->mapToGlobal(pos);
    QMenu myMenu;
    myMenu.addAction("删除", this, &MainWindow::removeDeviceItem);
    myMenu.exec(globalPos);
}
void MainWindow::removeDeviceItem() {
    QMutexLocker locker(&Global::mutex);
    int index = ui->listDevice->currentRow();
    if (index < 0) {
        return;
    }
    auto device = Global::dglabList[index];
    if(device->getBLEController()->state() != QLowEnergyController::ControllerState::UnconnectedState) {
        QMessageBox::warning(this, "通知", "您不能移除一个已经连接的设备，请关闭设备后再试。");
        return;
    }
    QListWidgetItem *item = ui->listDevice->takeItem(index);
    QListWidgetItem *itemOperator = ui->listOperator->takeItem(index);
    Global::dglabList.removeAt(index);
    delete item;
    delete itemOperator;
    delete device;
    ui->listDevice->update();
    saveDevice();
}
void MainWindow::showEvent(QShowEvent*) {
    //
}
void MainWindow::closeEvent(QCloseEvent*) {

}
void MainWindow::saveDevice() {
    QFile saveFile(QStringLiteral("save.json"));
    if (!saveFile.open(QIODevice::WriteOnly)) {
        return;
    }
    QJsonObject savedObject;
    savedObject["platform"] =
        #ifdef Q_OS_MACOS
            "macos"
        #else
            "compatible"
        #endif
    ;
    QJsonArray deviceArray;
    for (auto device: (Global::dglabList)) {
        QJsonObject bleObject;
        bleObject["address"] = device->getDeviceAddressOrUuid();
        bleObject["id"] = device->getID();
        deviceArray.append(bleObject);
    }
    savedObject["devices"] = deviceArray;
    QJsonDocument saveDoc(savedObject);
    saveFile.write(saveDoc.toJson());
}
void MainWindow::loadDevice() {
    libopendglab_ExportedSymbols *core = libopendglab_symbols();
    auto instance = core->kotlin.root.OpenDGLab.Device.Companion._instance();
    auto name = QString::fromUtf8(
                core->kotlin.root.OpenDGLab.Device.Companion.getName(
                        instance
                    )
                );
    QFile loadFile(QStringLiteral("save.json"));
    if (!loadFile.open(QIODevice::ReadOnly)) {
        return;
    }
    QByteArray savedDevice = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(savedDevice));
    QJsonObject json = loadDoc.object();
    QString platform = json["platform"].toString();
#ifdef Q_OS_MACOS
    if (platform == "compatible")
#else
    if (platform == "macos")
#endif
    {
        return;
    }
    QJsonArray deviceArray = json["devices"].toArray();
    for (auto array: deviceArray) {
        QJsonObject obj = array.toObject();
        QString address = obj["address"].toString();
        QString id = obj["id"].toString();
        QBluetoothDeviceInfo info = QBluetoothDeviceInfo(
        #ifdef Q_OS_MACOS
                            QBluetoothUuid(address)
        #else
                            QBluetoothAddress(address)
        #endif
        , name, 0);
        DGLabDevice *device = new DGLabDevice(info);
        Global::dglabList.append(device);
    }
    this->refreshDeviceList();
}
void MainWindow::refreshDeviceList() {
    for (auto device: Global::dglabList) {
        if (!device->isShownOnUi()) {
            device->setShownOnUi();
            auto listDevice = new QListWidgetItem(ui->listDevice);
            listDevice->setSizeHint(device->getUiDeviceItem()->sizeHint());
            ui->listDevice->addItem(listDevice);
            ui->listDevice->setItemWidget(listDevice, device->getUiDeviceItem());
            auto listOperator = new QListWidgetItem(ui->listOperator);
            listOperator->setSizeHint(device->getUiDeviceOperator()->sizeHint());
            ui->listOperator->addItem(listOperator);
            ui->listOperator->setItemWidget(listOperator, device->getUiDeviceOperator());
        }
    }
    saveDevice();
}
void MainWindow::allBoostChanged(int value) {
    ui->lbl_boost->setText(QString::number(value));
}

void MainWindow::startAllBoost(int boost)
{
    ui->allBoost->setEnabled(false);
    mvcController->startAllBoost(boost);
}

void MainWindow::stopAllBoost()
{
    ui->allBoost->setEnabled(true);
    mvcController->stopAllBoost();
}
