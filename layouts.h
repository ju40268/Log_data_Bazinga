#pragma once

#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QStackedLayout>
#include <string>
#include <map>
#include "devio.h"
#include "globalsettings.h"
#include "device.h"    // for ErrorInfo

class Device;

using devio::IDeviceManager;
using devio::IDevice;
using devio::Subscriber;

class Layouts :
        public QWidget,
        Subscriber<IDeviceManager::ArrivalRemoval>,
        Subscriber<IDevice::Connect>,
        Subscriber<IDeviceManager::RunLoopEvent>
{
    Q_OBJECT

public:
    Layouts(QWidget *parent = 0);

	void showerror(Device *dev = nullptr);
    void updateErrorDisplay();
    void enablePolling(bool enabled);
    bool isPolling() {return timer_refresh_devices != nullptr; }

    virtual void onRunLoopEvent() override;
    virtual void onDeviceRemoval(shared_ptr<IDevice> dev) override;
    virtual void onDeviceArrival(shared_ptr<IDevice> dev) override;
    virtual void onConnect(shared_ptr<IDevice> dev, bool settings_retained) override;
    virtual void onDisconnect(shared_ptr<IDevice> dev) override;
    virtual void onFirmwareChanged(shared_ptr<IDevice> dev) override;

    void onConfigChange(shared_ptr<IDevice> dev);

public slots:
    void onCurrentDeviceChanged(QListWidgetItem * current, QListWidgetItem * previous);
    void onDeviceSelectionChanged();
    void contextualMenu(const QPoint& point);
    void slotDeviceReset();

signals:
    void signalRunLoopEvent();
    void signalDeviceRemoval(shared_ptr<IDevice> dev);
    void signalDeviceArrival(shared_ptr<IDevice> dev);
    void signalConnect(shared_ptr<IDevice> dev, bool settings_retained);
    void signalDisconnect(shared_ptr<IDevice> dev);

private slots:
    void slotRunLoopEvent();
    void slotDeviceRemoval(shared_ptr<IDevice> dev);
    void slotDeviceArrival(shared_ptr<IDevice> dev);
    void slotConnect(shared_ptr<IDevice> dev, bool settings_retained);
    void slotDisconnect(shared_ptr<IDevice> dev);
    void slotAbandonDevices();
    void slotResetDevices();
    void slotRefreshDevices();

public:
    QLabel *status_label;
    QStackedLayout* sl;
    GlobalSettings* globalsettings_tab;
private:
    QListWidget *dlw;
    QStackedLayout* feature_stack;
    QStackedLayout* config_stack;
    QListWidget *flw;
    int currow;
    std::vector<Device *> devlist;
    bool reading_features;
    ErrorInfo global_errinfo;           // error info for global error (non device specific)
    QTimer* timer_refresh_devices;      // on OS X, we have to periodically refresh the device list to poll for Bluetooth device state
                                        // cdevio.Devices() as a side effect refreshes the state of any Bluetooth devices

private:
    // to help with debugging
    void dumpDevices();
};

std::string describe_err(devio::ErrorCode errcode,
                         unsigned long errcode_sys,
                         const char* errstr);
std::string describe_err(bool bresult);
std::string describe_err();

extern Layouts *g_layouts;
