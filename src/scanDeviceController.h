#ifndef SCANDEVICECONTROLLER_H
#define SCANDEVICECONTROLLER_H

#include <QObject>
#include <QBluetoothDeviceInfo>

class ScanDevice;
class ScanDeviceController : public QObject
{
    Q_OBJECT
public:
    explicit ScanDeviceController(QObject *parent = nullptr);
    ~ScanDeviceController() override;
    void startScan();
    void stopScan();

private slots:
    void deviceScanFound(const QBluetoothDeviceInfo&);
    void deviceScanError();
    void deviceScanFinished();

signals:
    void eventScanError();
    void eventScanFinished();
    void eventScanFound();
};

#endif // SCANDEVICECONTROLLER_H
