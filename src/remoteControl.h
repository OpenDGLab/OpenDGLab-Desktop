#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include <QWidget>

namespace Ui {
class RemoteControl;
}

class RemoteControl : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteControl(QWidget *parent = nullptr);
    ~RemoteControl() override;

private slots:
    void updateList();

private:
    Ui::RemoteControl *ui;
    void changeState(bool start);
};

#endif // REMOTECONTROL_H
