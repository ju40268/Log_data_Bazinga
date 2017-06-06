#pragma once

#include <QTabWidget>
#include <QListWidget>
#include <QTextEdit>
#include "devio.h"

extern bool g_showUsbDevices;
extern bool g_showUnifyingDevices;
extern bool g_showBluetoothDevices;
extern bool g_activeSession;
extern bool g_heavyConnect;

using devio::Subscriber;
using devio::ErrorCode;

class Layouts;

class GlobalSettings : public QTabWidget,
        public Subscriber<devio::IDeviceManager::UnreportedError>

{
    Q_OBJECT
public:
    explicit GlobalSettings(Layouts *lay);
    void onUnreportedError(const char *source, ErrorCode err, const char *str, unsigned long sys) override;
    void appendToLog(const QString &text);

private:
    QListWidgetItem *itemUsb;
    QListWidgetItem *itemUnifying;
    QListWidgetItem *itemBluetooth;
    QListWidgetItem *itemBle;
    QListWidgetItem *itemBleMonitor;
    QListWidgetItem *itemBleDisableCccd;
    QListWidgetItem *itemBlepp;
    QListWidgetItem *itemActive;
    QListWidgetItem *itemPoll;
    QListWidgetItem *itemPersist;
    QListWidgetItem *itemHeavyConnect;
    QListWidgetItem *itemUnifyingBrb;

    QListWidget *page_settings;
    QTextEdit *log;

    Layouts *lay;

public slots:
    void onContextMenu(const QPoint &pos);
    void onResetToDefault(void);
    void onResetToDevioDefault(void);
    void onResetToDefaultInternal(bool reset, bool showdevices);

signals:
    void signalOnUnreportedError(const char *source, ErrorCode err, const char *str, unsigned long sys);

private slots:
    void onSettingChanged(QListWidgetItem*item);
    void slotOnUnreportedError(const char *source, ErrorCode err, const char *str, unsigned long sys);
};
