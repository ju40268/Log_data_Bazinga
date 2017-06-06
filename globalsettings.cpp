#include "feature.h"
#include "globalsettings.h"
#include <sstream>
#include <iomanip>
#include "devio.h"
#include "stringbuilder.h"
#include "layouts.h"
#include <QTimer>
#include <QDir>
#include <QSettings>
#include <QMenu>

using devio::cdevio;
using devio::Delivery;
using devio::ErrorCode;

bool g_showUsbDevices = false;
bool g_showUnifyingDevices = false;
bool g_showBluetoothDevices = false;
bool g_showBleDevices = false;
bool g_unifyingBrb = false;
bool g_useBlepp = false;
bool g_monitorBle = false;
bool g_bleDisableCccd = false;
bool g_activeSession = false;
bool g_heavyConnect = false;
bool g_usePersistentDataFile = false;

GlobalSettings::GlobalSettings(Layouts *layout) :
    QTabWidget(),
    lay(layout)
{
    log = new QTextEdit;
    log->setPlainText("Global Settings");
    log->setReadOnly(true);
    log->setWordWrapMode(QTextOption::NoWrap);

    qRegisterMetaType<ErrorCode>("ErrorCode");
    if (!connect(this, SIGNAL(signalOnUnreportedError(const char *, ErrorCode, const char *, unsigned long)), 
                 this, SLOT(slotOnUnreportedError(const char *, ErrorCode, const char *, unsigned long)), 
                 Qt::QueuedConnection))
    {
        log->append("Error: signalOnUnreportedError to slotOnUnreportedError not connected!");
    }

    bool bresult = subscribe(cdevio,Delivery::Immediate);
    log->append(stringbuilder()
        << "Subscribing to devio errors -> "
        << describe_err(bresult));

    page_settings = new QListWidget;

    // update page_divert
    page_settings->setSelectionMode(QAbstractItemView::NoSelection);

    itemUsb = new QListWidgetItem("Show usb devices");
    page_settings->addItem(itemUsb);

    itemUnifying = new QListWidgetItem("Show unifying devices");
    page_settings->addItem(itemUnifying);

    itemBluetooth = new QListWidgetItem("Show bluetooth classic devices");
    page_settings->addItem(itemBluetooth);

    itemBle = new QListWidgetItem("Show bluetooth low energy devices");
    page_settings->addItem(itemBle);

    itemBleMonitor = new QListWidgetItem("Monitor other ble interface");
    page_settings->addItem(itemBleMonitor);

    itemBlepp = new QListWidgetItem("Use blepp interface");
    page_settings->addItem(itemBlepp);          

    itemBleDisableCccd = new QListWidgetItem("On blepp disable cccd");
    page_settings->addItem(itemBleDisableCccd);

    itemPoll = new QListWidgetItem("Poll for bluetooth unpairing");
    page_settings->addItem(itemPoll);

    itemPersist = new QListWidgetItem("Use Persistent Data Directory:  ./BazingaCache");
    page_settings->addItem(itemPersist);

    itemActive = new QListWidgetItem("Active Session");
    page_settings->addItem(itemActive);

    itemHeavyConnect = new QListWidgetItem("Reload features on connect");
    page_settings->addItem(itemHeavyConnect);

    itemUnifyingBrb = new QListWidgetItem("Unifying buffered response bug workaround");
    page_settings->addItem(itemUnifyingBrb);

    log->append("connecting onSettingChanged");
    if (!connect(page_settings, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onSettingChanged(QListWidgetItem*))))
    {
        log->append("Error: Signal itemChanged to slot onSettingChanged not connected!");
    }

    addTab(page_settings,"Global Settings");
    addTab(log,"Log");

    onResetToDefaultInternal(false, true);

    // right click to "Change all settings to Default"
    page_settings->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(page_settings, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenu(const QPoint&)));
}

void GlobalSettings::onContextMenu(const QPoint &pos)
{
    (void) pos;
    QMenu *menu = new QMenu;
    menu->addAction(QString("Change all settings to default values"), this, SLOT(onResetToDefault()));
    menu->addAction(QString("Devio defaults (will hide devices until you select busses)"), this, SLOT(onResetToDevioDefault()));
    menu->exec(QCursor::pos());
}

void GlobalSettings::onResetToDefault()
{
    onResetToDefaultInternal(true,true);
}

void GlobalSettings::onResetToDevioDefault()
{
    onResetToDefaultInternal(true,false);
}

void GlobalSettings::onResetToDefaultInternal(bool reset, bool showdevices)
{
    // defaults
    bool showUsb = false;
    bool showUnifying = false;
    bool showBluetooth = false;
    bool showBle = false;
    bool useBlepp = false;
    bool monitorBle = false;
    bool bPoll = false;
    bool persistentData = false;
    bool heavyConnect = false;
    bool activeSession = true;
    bool bleDisableCccd = true;
    bool unifyingBrb = true;

    // get devio defaults
    cdevio.config_default(devio::ConfigItem::Usb, &showUsb);
    cdevio.config_default(devio::ConfigItem::Unifying, &showUnifying);
    cdevio.config_default(devio::ConfigItem::Bluetooth, &showBluetooth);
    cdevio.config_default(devio::ConfigItem::Ble, &showBle);
    cdevio.config_default(devio::ConfigItem::Blepp, &useBlepp);
    cdevio.config_default(devio::ConfigItem::BleMonitor, &monitorBle);
    cdevio.config_default(devio::ConfigItem::BleDisableCccd, &bleDisableCccd);
    cdevio.config_default(devio::ConfigItem::UnifyingBrb, &unifyingBrb);

    // override devio defaults with useful settings for bazinga
    if (showdevices)
    {
        showUsb = true;
        showUnifying = true;
        showBluetooth = true;
        showBle = true;
    }

    // get persistent settings
    if (!reset)
    {
        QSettings settings;
        showUsb        = (settings.value("usb"         , showUsb        ? "yes" : "no").toString() == "yes");
        showUnifying   = (settings.value("unifying"    , showUnifying   ? "yes" : "no").toString() == "yes");
        showBluetooth  = (settings.value("bluetoooth"  , showBluetooth  ? "yes" : "no").toString() == "yes");
        showBle        = (settings.value("ble"         , showBle        ? "yes" : "no").toString() == "yes");
        monitorBle     = (settings.value("blemonitor"  , monitorBle     ? "yes" : "no").toString() == "yes");
        useBlepp       = (settings.value("blepp"       , useBlepp       ? "yes" : "no").toString() == "yes");
        bPoll          = (settings.value("polling"     , bPoll          ? "yes" : "no").toString() == "yes");
        persistentData = (settings.value("persistent"  , persistentData ? "yes" : "no").toString() == "yes");
        heavyConnect   = (settings.value("heavyconnect", heavyConnect   ? "yes" : "no").toString() == "yes");
        unifyingBrb    = (settings.value("unifyingbrb" , unifyingBrb    ? "yes" : "no").toString() == "yes");
    }

    // mark checkboxes to defaults and act on any changes
    itemUsb         ->setCheckState( showUsb        ? Qt::Checked : Qt::Unchecked);
    itemUnifying    ->setCheckState( showUnifying   ? Qt::Checked : Qt::Unchecked);
    itemBluetooth   ->setCheckState( showBluetooth  ? Qt::Checked : Qt::Unchecked);
    itemBle         ->setCheckState( showBle        ? Qt::Checked : Qt::Unchecked);
    itemBleMonitor  ->setCheckState( monitorBle     ? Qt::Checked : Qt::Unchecked);
    itemBleDisableCccd->setCheckState( bleDisableCccd ? Qt::Checked : Qt::Unchecked);
    itemBlepp       ->setCheckState( useBlepp       ? Qt::Checked : Qt::Unchecked);
    itemPoll        ->setCheckState( bPoll          ? Qt::Checked : Qt::Unchecked);
    itemPersist     ->setCheckState( persistentData ? Qt::Checked : Qt::Unchecked);
    itemActive      ->setCheckState( activeSession  ? Qt::Checked : Qt::Unchecked);
    itemHeavyConnect->setCheckState( heavyConnect   ? Qt::Checked : Qt::Unchecked);
    itemUnifyingBrb ->setCheckState( unifyingBrb    ? Qt::Checked : Qt::Unchecked);

    log->append( stringbuilder() << "Show Usb Devices = "       << g_showUsbDevices);
    log->append( stringbuilder() << "Show Unifying Devices = "  << g_showUnifyingDevices);
    log->append( stringbuilder() << "Show Bluetooth Devices = " << g_showBluetoothDevices);
    log->append( stringbuilder() << "Show Ble Devices = "       << g_showBleDevices);
    log->append( stringbuilder() << "Active Session = "         << g_activeSession);
}


void GlobalSettings::onSettingChanged(QListWidgetItem*item)
{
    QSettings settings;

//    unsigned int r = page_settings->row(item);
//    log->append(stringbuilder() << "setting change row is "<<r);

    if (item == itemUsb)
    {
        bool bCheckState = itemUsb->checkState();

        g_showUsbDevices = bCheckState;
        log->append(stringbuilder() << "Changed g_showUsbDevices to " << g_showUsbDevices);

        cdevio.config_set(devio::ConfigItem::Usb, g_showUsbDevices);
        //QTimer::singleShot(100, g_layouts, SLOT(slotResetDevices()));
        settings.setValue("usb",bCheckState ? "yes" : "no");
    }

    if (item == itemUnifying)
    {
        bool bCheckState = itemUnifying->checkState();

        g_showUnifyingDevices = bCheckState;
        log->append(stringbuilder() << "Changed g_showUnifyingDevices to " << g_showUnifyingDevices);

        cdevio.config_set(devio::ConfigItem::Unifying, g_showUnifyingDevices);
        //QTimer::singleShot(100, g_layouts, SLOT(slotResetDevices()));
        settings.setValue("unifying",bCheckState ? "yes" : "no");
    }

    if (item == itemBluetooth)
    {
        bool bBluetooth = itemBluetooth->checkState();

        g_showBluetoothDevices = bBluetooth;
        log->append(stringbuilder() << "Changed g_showBluetooothDevices to " << g_showBluetoothDevices);

        cdevio.config_set(devio::ConfigItem::Bluetooth, g_showBluetoothDevices);
        //QTimer::singleShot(100, g_layouts, SLOT(slotResetDevices()));
        settings.setValue("bluetoooth",bBluetooth ? "yes" : "no");
    }

    if (item == itemBle)
    {
        bool bCheckState = itemBle->checkState();

        g_showBleDevices = bCheckState;
        log->append(stringbuilder() << "Changed g_showBle to " << g_showBleDevices);

        cdevio.config_set(devio::ConfigItem::Ble, g_showBleDevices);
        //QTimer::singleShot(100, g_layouts, SLOT(slotResetDevices()));
        settings.setValue("ble",bCheckState ? "yes" : "no");
    }

    if (item == itemBleMonitor)
    {
        bool bCheckState = itemBleMonitor->checkState();

        g_monitorBle = bCheckState;
        log->append(stringbuilder() << "Changed g_monitorBle to " << g_monitorBle);

        cdevio.config_set(devio::ConfigItem::BleMonitor, g_monitorBle);
        //QTimer::singleShot(100, g_layouts, SLOT(slotResetDevices()));
        settings.setValue("blemonitor",bCheckState ? "yes" : "no");
    }

    if (item == itemBleDisableCccd)
    {
        bool bCheckState = itemBleDisableCccd->checkState();

        g_bleDisableCccd = bCheckState;
        log->append(stringbuilder() << "Changed g_monitorBle to " << g_bleDisableCccd);

        cdevio.config_set(devio::ConfigItem::BleDisableCccd, g_bleDisableCccd);
        //QTimer::singleShot(100, g_layouts, SLOT(slotResetDevices()));
        settings.setValue("bledisablecccd",bCheckState ? "yes" : "no");
    }

    if (item == itemBlepp)
    {
        bool bCheckState = itemBlepp->checkState();


        g_useBlepp = bCheckState;
        log->append(stringbuilder() << "Changed g_useBlepp to " << g_useBlepp);

        cdevio.config_set(devio::ConfigItem::Blepp, g_useBlepp);
        //QTimer::singleShot(100, g_layouts, SLOT(slotResetDevices()));
        settings.setValue("blepp",bCheckState ? "yes" : "no");
    }

    if (item == itemPersist)
    {
        bool bPersist = itemPersist->checkState();

        if (bPersist)
        {
            QDir().mkdir("BazingaCache");  // application-specific devio cache
        }

        g_usePersistentDataFile = bPersist;
        log->append(stringbuilder() << "Changed g_usePersistentDataFile to " << g_usePersistentDataFile);

        cdevio.persistent_data_directory(g_usePersistentDataFile ? "BazingaCache" : "");
        settings.setValue("persistent",bPersist ? "yes" : "no");

    }

    if (item == itemPoll)
    {
        bool bPoll = itemPoll->checkState();

        lay->enablePolling(bPoll);
        log->append(stringbuilder() << "Changed polling to " << bPoll);
        settings.setValue("polling",bPoll ? "yes" : "no");
    }

    if (item == itemActive)
    {
        bool bActive = itemActive->checkState();

        g_activeSession = bActive;
        log->append(stringbuilder() << "Changed g_activeSession to " << g_activeSession);

        cdevio.set_active(bActive);
    }

    if (item == itemHeavyConnect)
    {
        g_heavyConnect = itemHeavyConnect->checkState();
        log->append(stringbuilder() << "Changed g_heavyConnect to " << g_activeSession);
        settings.setValue("heavyconnect",g_heavyConnect ? "yes" : "no");
    }

    if (item == itemUnifyingBrb)
    {
        bool bCheckState = itemUnifyingBrb->checkState();

        g_unifyingBrb = bCheckState;
        log->append(stringbuilder() << "Changed g_unifyingBrb to " << g_unifyingBrb);

        cdevio.config_set(devio::ConfigItem::UnifyingBrb, g_unifyingBrb);
        //QTimer::singleShot(100, g_layouts, SLOT(slotResetDevices()));
        settings.setValue("unifyingbrb",bCheckState ? "yes" : "no");
    }
}

void GlobalSettings::onUnreportedError(const char *source, devio::ErrorCode err, const char *str, unsigned long sys)
{
    stringbuilder sb;

    emit signalOnUnreportedError(source, err, str, sys);
}

void GlobalSettings::slotOnUnreportedError(const char *source, devio::ErrorCode err, const char *str, unsigned long sys)
{
    devio::errcode = err;
    devio::errcode_str = str;
    devio::errcode_sys = sys;

    log->append(stringbuilder()
        << "Devio error: "
        << source
        << " - "
        << describe_err());
}

#include "features_all.h"
void GlobalSettings::appendToLog(const QString &text)
{
    QString str(FeaturesAll::timestamp_full(FeaturesAll::int_timestamp()).c_str());

    str += " - " + text;
    log->append(str);
}
