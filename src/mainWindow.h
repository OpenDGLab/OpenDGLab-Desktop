#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mainWindowController.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void saveDevice();
    void loadDevice();

protected:
    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;

private slots:
    void addDevice();
    void refreshDeviceList();
    void showContextMenu(const QPoint &);
    void removeDeviceItem();
    void allBoostChanged(int);
    void startAllBoost(int);
    void stopAllBoost();

private:
    Ui::MainWindow *ui;
    MainWindowController *mvcController;
};
#endif // MAINWINDOW_H
