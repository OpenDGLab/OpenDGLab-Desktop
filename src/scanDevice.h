#ifndef SCANDEVICE_H
#define SCANDEVICE_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QShowEvent>
#include "scanDeviceController.h"

namespace Ui {
class ScanDevice;
}

class ScanDevice : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScanDevice(QWidget *parent = nullptr);
    ~ScanDevice() override;

private:
    Ui::ScanDevice *ui;
    ScanDeviceController* mvcController;

public slots:
    void setCancelled();
    void setError();
    void setFinished();
    void setFound();

protected:
    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;

signals:
    void needRefresh();

};

#endif // SCANDEVICE_H
