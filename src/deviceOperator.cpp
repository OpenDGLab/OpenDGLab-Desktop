#include "deviceOperator.h"
#include "ui_deviceOperator.h"
#include "global.h"

DeviceOperator::DeviceOperator(QString id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceOperator)
{
    ui->setupUi(this);
    ui->lbl_id->setText(id);
    for (auto wave: Global::basicWaveNameList) {
        ui->cb_a_wave->addItem(wave);
        ui->cb_b_wave->addItem(wave);
    }
    ui->cb_a_wave->insertSeparator(ui->cb_a_wave->count());
    ui->cb_b_wave->insertSeparator(ui->cb_b_wave->count());
    for (auto wave: Global::touchWaveNameList) {
        ui->cb_a_wave->addItem(wave);
        ui->cb_b_wave->addItem(wave);
    }
    ui->cb_a_wave->insertSeparator(ui->cb_a_wave->count());
    ui->cb_b_wave->insertSeparator(ui->cb_b_wave->count());
    ui->cb_a_wave->addItem("外部输入");
    ui->cb_b_wave->addItem("外部输入");
    connect(ui->chk_a_auto, &QCheckBox::stateChanged, [this](int state){
        ui->cb_a_wave->setEnabled(!state);
        ui->sp_a_auto_timer->setEnabled(!state);
        if (!state) {
            autoATimer = 0;
        }
    });
    connect(ui->chk_b_auto, &QCheckBox::stateChanged, [this](int state){
        ui->cb_b_wave->setEnabled(!state);
        ui->sp_b_auto_timer->setEnabled(!state);
        if (!state) {
            autoBTimer = 0;
        }
    });
    connect(ui->silde_a_step, &QSlider::valueChanged, [this](int value){
        ui->lbl_a_step->setText(QString::number(value));
    });
    connect(ui->silde_b_step, &QSlider::valueChanged, [this](int value){
        ui->lbl_b_step->setText(QString::number(value));
    });
    connect(ui->sb_a_boost, &QSlider::valueChanged, [this](int value){
        ui->lbl_a_boost->setText(QString::number(value));
    });
    connect(ui->sb_b_boost, &QSlider::valueChanged, [this](int value){
        ui->lbl_b_boost->setText(QString::number(value));
    });

    connect(ui->btn_a_increase, &QPushButton::clicked, [this](bool){
        emit changePower(ui->lcd_a_strength->intValue() + ui->silde_a_step->value(), ui->lcd_b_strength->intValue());
    });
    connect(ui->btn_a_decrease, &QPushButton::clicked, [this](bool){
        emit changePower(ui->lcd_a_strength->intValue() - ui->silde_a_step->value(), ui->lcd_b_strength->intValue());
    });
    connect(ui->btn_a_stop, &QPushButton::clicked, [this](bool){
        emit changePower(0, ui->lcd_b_strength->intValue());
    });
    connect(ui->btn_b_increase, &QPushButton::clicked, [this](bool){
        emit changePower(ui->lcd_a_strength->intValue(), ui->lcd_b_strength->intValue() + ui->silde_b_step->value());
    });
    connect(ui->btn_b_decrease, &QPushButton::clicked, [this](bool){
        emit changePower(ui->lcd_a_strength->intValue(), ui->lcd_b_strength->intValue() - ui->silde_b_step->value());
    });
    connect(ui->btn_b_stop, &QPushButton::clicked, [this](bool){
        emit changePower(ui->lcd_a_strength->intValue(), 0);
    });

    connect(ui->cb_a_wave, &QComboBox::currentTextChanged, [this](const QString & value){
        emit changeWave(DeviceStateEnum::DeviceChannel::CHANNEL_A, value);
    });
    connect(ui->cb_b_wave, &QComboBox::currentTextChanged, [this](const QString & value){
        emit changeWave(DeviceStateEnum::DeviceChannel::CHANNEL_B, value);
    });
    connect(ui->btn_a_boost, &QPushButton::pressed, [this](){
        ui->sb_a_boost->setEnabled(false);
        emit startBoost(DeviceStateEnum::DeviceChannel::CHANNEL_A, ui->sb_a_boost->value());
    });
    connect(ui->btn_b_boost, &QPushButton::pressed, [this](){
        ui->sb_b_boost->setEnabled(false);
        emit startBoost(DeviceStateEnum::DeviceChannel::CHANNEL_B, ui->sb_b_boost->value());
    });
    connect(ui->btn_a_boost, &QPushButton::released, [this](){
        ui->sb_a_boost->setEnabled(true);
        emit stopBoost(DeviceStateEnum::DeviceChannel::CHANNEL_A, ui->sb_a_boost->value());
    });
    connect(ui->btn_b_boost, &QPushButton::released, [this](){
        ui->sb_b_boost->setEnabled(true);
        emit stopBoost(DeviceStateEnum::DeviceChannel::CHANNEL_B, ui->sb_b_boost->value());
    });
}

DeviceOperator::~DeviceOperator()
{
    delete ui;
}

QString DeviceOperator::getWaveA()
{
    return ui->cb_a_wave->currentText();
}
QString DeviceOperator::getWaveB()
{
    return ui->cb_b_wave->currentText();
}

void DeviceOperator::setWaveA(QString str) {
    if (ui->cb_a_wave->findText(str) >= 0) {
        ui->cb_a_wave->setCurrentText(str);
    }
}
void DeviceOperator::setWaveB(QString str) {
    if (ui->cb_b_wave->findText(str) >= 0) {
        ui->cb_b_wave->setCurrentText(str);
    }
}

void DeviceOperator::checkIfNeedChangeWave()
{
    if (ui->chk_a_auto->isChecked()) {
        autoATimer++;
        if (autoATimer > ui->sp_a_auto_timer->value()) {
            auto next = random.bounded(0, ui->cb_a_wave->count());
            ui->cb_a_wave->setCurrentIndex(next);
            autoATimer = 0;
        }
    }
    if (ui->chk_b_auto->isChecked()) {
        autoBTimer++;
        if (autoBTimer > ui->sp_b_auto_timer->value()) {
            auto next = random.bounded(0, ui->cb_b_wave->count());
            ui->cb_b_wave->setCurrentIndex(next);
            autoBTimer = 0;
        }
    }
}

void DeviceOperator::setPower(DeviceStateEnum::DeviceChannel channel, int level) {
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
        ui->lcd_a_strength->display(level);
        break;
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
        ui->lcd_b_strength->display(level);
        break;
    }
}
void DeviceOperator::setAutoChange(DeviceStateEnum::DeviceChannel channel, bool autoChange) {
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
        ui->chk_a_auto->setChecked(autoChange);
        break;
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
        ui->chk_b_auto->setChecked(autoChange);
        break;
    }
}
bool DeviceOperator::getAutoChange(DeviceStateEnum::DeviceChannel channel) {
    switch (channel) {
    case DeviceStateEnum::DeviceChannel::CHANNEL_A:
        return ui->chk_a_auto->isChecked();
    case DeviceStateEnum::DeviceChannel::CHANNEL_B:
        return ui->chk_b_auto->isChecked();
    }
    return false;
}
