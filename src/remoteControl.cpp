#include "remoteControl.h"
#include "ui_remoteControl.h"
#include "global.h"
#include <QMessageBox>

RemoteControl::RemoteControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RemoteControl)
{
    ui->setupUi(this);
    ui->btn_remote_remove->setEnabled(false);
    changeState(false);
    connect(ui->chk_remote, &QCheckBox::stateChanged, this,[this](int state){
        changeState(state);
    });
    connect(ui->sb_tcp_port, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
       Global::remote->setTcpPort(value);
    });
    connect(ui->sb_ws_port, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
        Global::remote->setWSPort(value);
    });
    connect(ui->listWidget, &QListWidget::currentTextChanged, this, [this](const QString &currentText){
        if (currentText.isEmpty()) {
            ui->lbl_title->setText("");
            ui->lbl_remote_id->setText("");
            ui->btn_remote_remove->setEnabled(false);
        } else {
            QString uuid;
            uuid = currentText.split("(")[1];
            uuid = uuid.split(")")[0];
            auto name = Global::remote->preAuthName[uuid];
            ui->lbl_title->setText(name);
            ui->lbl_remote_id->setText(uuid);
            ui->btn_remote_remove->setEnabled(true);
        }
    });
    connect(ui->btn_remote_remove, &QPushButton::clicked, this, [this](){
        RemoteClient* client = nullptr;
        Global::remote->preAuth.remove(ui->lbl_remote_id->text());
        Global::remote->preAuthName.remove(ui->lbl_remote_id->text());
        for(auto r: Global::remoteList) {
            if (r->getUuid() == ui->lbl_remote_id->text()) {
                client = r;
                break;
            }
        }
        if (client != nullptr){
            client->close();
            Global::remote->save();
        }
        updateList();
    });
    connect(Global::remote, &Remote::stateChange, this, &RemoteControl::updateList);
    updateList();
}

void RemoteControl::changeState(bool start) {
    ui->sb_tcp_port->setEnabled(!start);
    ui->sb_ws_port->setEnabled(!start);
    ui->listWidget->setEnabled(start);
    ui->btn_remote_remove->setEnabled(start);
    if (start) {
        if(!Global::remote->start()) {
            QMessageBox::warning(this, "警告", "无法启动服务器。");
            changeState(false);
        }
    } else {
        Global::remote->stop();
        ui->listWidget->clearSelection();
        ui->btn_remote_remove->setEnabled(false);
    }
}

RemoteControl::~RemoteControl()
{
    delete ui;
}

void RemoteControl::updateList() {
    ui->listWidget->clearSelection();
    ui->listWidget->clear();
    for (const auto& pre: Global::remote->preAuthName.keys()){
        auto k = Global::remote->preAuthName[pre];
        ui->listWidget->addItem(QString("%1 (%2)").arg(k, pre));
    }
}
