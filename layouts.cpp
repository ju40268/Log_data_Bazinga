#include "feature.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include "layouts.h"
#include "Device.h"
#include "Feature.h"
#include "devio.h"
#include "devio_strings.h"
#include <sstream>
#include <iomanip>
#include <QApplication>
#include <QTextEdit>
#include <QStackedLayout>
#include "stringbuilder.h"
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QTimer>
#include <QMenu>
#include "xlog/xlog.h"
using devio::cdevio;
using std::string;

// fix selection foreground colors on mac
#include <QItemDelegate>
class MyItemDelegate: public QItemDelegate
{
public:
    MyItemDelegate(QObject* pParent = 0) : QItemDelegate(pParent)
    {
    }
    
    void paint(QPainter* pPainter, const QStyleOptionViewItem& rOption, const QModelIndex& rIndex) const
    {
        QStyleOptionViewItem ViewOption(rOption);
        
        QColor ItemForegroundColor = rIndex.data(Qt::ForegroundRole).value<QColor>();
        if (ItemForegroundColor.isValid())
        {
            if (ItemForegroundColor != rOption.palette.color(QPalette::WindowText))
            {
                ViewOption.palette.setColor(QPalette::HighlightedText, ItemForegroundColor);
            }
        }
        QItemDelegate::paint(pPainter, ViewOption, rIndex);
    }
};

Layouts::Layouts(QWidget *parent) :
    QWidget(parent),
    currow(-1),
    reading_features(false),
    timer_refresh_devices(nullptr)
{
    Info("Layouts::Layouts");

    // need to register this to use this type in a signal
    qRegisterMetaType<shared_ptr<IDevice>>("shared_ptr<IDevice>");

    if (connect(this, SIGNAL(signalRunLoopEvent()),
            this, SLOT(slotRunLoopEvent()),
            Qt::QueuedConnection))
    {
        Info("Connected RunLoopEvent signal and slot");
    }
    else
    {
        Info("Failed to connect RunLoopEvent signal and slot");
    }

    connect(this, SIGNAL(signalDeviceRemoval(shared_ptr<IDevice>)),
            this, SLOT(slotDeviceRemoval(shared_ptr<IDevice>)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(signalDeviceArrival(shared_ptr<IDevice>)),
            this, SLOT(slotDeviceArrival(shared_ptr<IDevice>)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(signalConnect(shared_ptr<IDevice>,bool)),
            this, SLOT(slotConnect(shared_ptr<IDevice>,bool)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(signalDisconnect(shared_ptr<IDevice>)),
            this, SLOT(slotDisconnect(shared_ptr<IDevice>)),
            Qt::QueuedConnection);

    resize(780,500);

    QVBoxLayout *vboxl = new QVBoxLayout();
    QVBoxLayout *vboxr = new QVBoxLayout();
    QHBoxLayout *hbox = new QHBoxLayout(this);

    dlw = new QListWidget(this);

#ifdef Q_OS_MAC
    // qt's mac style obscures the foreground color on selected items, so preserve foreground color
    dlw->setItemDelegate(new MyItemDelegate(this));
#endif
    
    status_label = new QLabel("", this);
    QLabel *features_label = new QLabel("Features:", this);
    feature_stack = new QStackedLayout();
    config_stack = new QStackedLayout();

    if (!Subscriber<RunLoopEvent>::subscribe(cdevio,
                                               devio::Delivery::Immediate,
                                               "Layouts::RunLoopEvent")) 
	{
        globalsettings_tab->appendToLog("error subscribing to RunLoopEvent events");
		showerror();
	}

    // left side
    vboxl->addWidget(new QLabel("Devices:", this));
    vboxl->addWidget(dlw,2);
    vboxl->addWidget(features_label);
    vboxl->addLayout(feature_stack,5);

    // right side
    vboxr->addWidget(status_label);
    vboxr->addLayout(config_stack);

    hbox->addLayout(vboxl,2);
    hbox->addSpacing(15);
    hbox->addLayout(vboxr,5);

    setLayout(hbox);

    // fake device that contains global settings UI
    auto item = new QListWidgetItem("All Devices");
    dlw->addItem(item);
    QListWidget *featurelist = new QListWidget(this);
    feature_stack->addWidget(featurelist);

    globalsettings_tab = new GlobalSettings(this);
    config_stack->addWidget(globalsettings_tab);

    devlist.push_back(nullptr);

    connect(dlw, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(onCurrentDeviceChanged(QListWidgetItem *, QListWidgetItem *)));
    connect(dlw, SIGNAL(itemSelectionChanged()),
            this, SLOT(onDeviceSelectionChanged()));

    QTimer::singleShot(100, this, SLOT(slotResetDevices()));
}

void Layouts::enablePolling(bool enable)
{
    if (enable)
    {
        if (!timer_refresh_devices)
        {
            timer_refresh_devices = new QTimer(this);
            connect(timer_refresh_devices, SIGNAL(timeout()), this, SLOT(slotRefreshDevices()));
            timer_refresh_devices->start(1000 /* milliseconds*/);
        }
    }
    else
    {
        if (timer_refresh_devices)
        {
            timer_refresh_devices->stop();
            disconnect(timer_refresh_devices, SIGNAL(timeout()), this, SLOT(slotRefreshDevices()));

            delete timer_refresh_devices;
            timer_refresh_devices = nullptr;
        }
    }
}

void Layouts::slotResetDevices()
{
    Info("Layouts::slotResetDevices() begin...");

    cdevio.swid(0xF);  // make Bazinga always use swid 0xF to better test swid collision handling
                
    if (!Subscriber<ArrivalRemoval>::subscribe(cdevio,
                                               devio::Delivery::RunLoop,
                                               "Layouts::ArrivalRemoval"))
    {
        globalsettings_tab->appendToLog("error subscribing to ArrivalRemovalEvents");
        showerror();
    }

    feature_stack->setCurrentIndex(0);
    config_stack->setCurrentIndex(0);

    while (devlist.size() > 1)
    {
        slotDeviceRemoval(devlist[1]->device());
    }

    auto devs = cdevio.devices();
    for (auto dev : devs)
    {
        slotDeviceArrival(dev);
    }

	if (devs.empty() && devio::is_err())
	{
        globalsettings_tab->appendToLog("error enumerating devices");
		showerror();
	}

    if (currow >= 0)
    {
        onDeviceSelectionChanged();
    }

    // enablePolling(true);

    Info("Layouts::slotResetDevices() ...end");
}

void Layouts::slotRefreshDevices()
{
    Debug("Layouts::slotRefreshDevices()");

    // As a side effect, this causes the MacOS hardware bus to refresh itself. Any BT
    // device that has been unpaired since the last time we enumerated devices will cause
    // a device removal event.

    cdevio.devices();
}

void Layouts::slotAbandonDevices()
{
    Info("Layouts::slotAbandonDevices() begin...");
    enablePolling(false);
    
//  Devio does cleanup automatically now, so this is not needed any more...
//
//    for (auto dev : cdevio.devices())
//    {
//        //dev->abandon();
//        dev->shutdown();
//    }

    Info("Layouts::slotAbandonDevices() ...end");
}

void Layouts::onRunLoopEvent()
{
    Info("Layouts::onRunLoopEvent");
    emit signalRunLoopEvent();
}

void Layouts::slotRunLoopEvent()
{
    Info("Layouts::slotRunLoopEvent");
    cdevio.call_event_from_runloop();
}

void Layouts::onDeviceRemoval(shared_ptr<IDevice> dev)
{
    Info("Layouts::onDeviceRemoval(pid = 0x%x)", dev->pid());

    // We are not in the gui thread.. send notification to gui thread.
    emit signalDeviceRemoval(dev);
}

void Layouts::onDeviceArrival(shared_ptr<IDevice> dev)
{
    Info("Layouts::onDeviceArrival(pid = 0x%x)", dev->pid());
    
    // We are not in the gui thread.. send notification to gui thread.
    emit signalDeviceArrival(dev);
}

void Layouts::onConnect(shared_ptr<IDevice> dev, bool settings_retained)
{
    // We are not in the gui thread.. send notification to gui thread.
    emit signalConnect(dev, settings_retained);
}

void Layouts::onConfigChange(shared_ptr<IDevice> dev)
{
    int i = 0;
    for (auto existing : devlist)
    {
        if (existing && existing->device_ == dev)
        {
            if (dlw->count() > i)
            {
                bool connected = dev->connected();
                bool configured = dev->configured();

                if (connected && configured)
                {
                    dlw->item(i)->setForeground(Qt::red);
                }
                else if (connected)
                {
                    dlw->item(i)->setForeground(Qt::black);
                }
                else if (configured)
                {
                    dlw->item(i)->setForeground(QColor(0xFF,0xA0,0xA0));     // gray red
                }
                else
                {
                    dlw->item(i)->setForeground(Qt::gray);
                }

                dlw->viewport()->update();
            }

        }

        i++;
    }
}

void Layouts::onDisconnect(shared_ptr<IDevice> dev)
{
    // We are not in the gui thread.. send notification to gui thread.
    emit signalDisconnect(dev);
}

void Layouts::onFirmwareChanged(shared_ptr<IDevice> dev)
{
    Info("Layouts::onFirmwareChanged()");

    unsigned int row = dlw->currentRow();

    for (unsigned int i=0; i < devlist.size(); ++i)
    {
        auto device = devlist[i];
        if (device && device->device() == dev)
        {
            devlist[row]->forget();

            if (row == i)
            {
                devlist[row]->startup();
            }
        }
    }
}

void Layouts::slotDeviceArrival(shared_ptr<IDevice> dev)
{
    Device *fl = new Device(this,dev);

    string deviceName = fl->getDeviceName();
    Info("Layouts::slotDeviceArrival, pid=%0x", dev->pid());

    globalsettings_tab->appendToLog(stringbuilder() << "device arrived: " << dev->name());
                                    
    auto item = new QListWidgetItem(stringbuilder() << deviceName);
    item->setForeground(dev->connected() ? Qt::black : Qt::gray);

    dlw->addItem(item);
    if (!Subscriber<Connect>::subscribe(dev))
    {
        globalsettings_tab->appendToLog(stringbuilder() << "error subscribing to Connect events for device: " << deviceName);
        fl->showerror();
    }

    feature_stack->addWidget(fl);

    // context menu on feature list for this device
    fl->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(fl,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(contextualMenu(const QPoint &)));

    config_stack->addWidget(fl->sl_holder);
    devlist.push_back(fl);

    dumpDevices();
    updateErrorDisplay();

    if (dev->connected())
    {
        slotConnect(dev,false);
    }

    Info("Layouts::slotDeviceArrival exit, pid=%0x", dev->pid());
}

void Layouts::slotDeviceRemoval(shared_ptr<IDevice> dev)
{
    Info("Layouts::slotDeviceRemoval, pid=%0x", dev->pid());

    globalsettings_tab->appendToLog(stringbuilder() << "device removed: " << dev->name());

    for (unsigned int i=0; i < devlist.size(); ++i)
    {
        auto device = devlist[i];
        if (device && device->device() == dev)
        {
            if ((int)i == dlw->currentRow())
            {
                // selected row removed, so jump to row 0 so we don't talk to the next device
                dlw->setCurrentRow(0);
            }

            delete config_stack->takeAt(i);
            delete feature_stack->takeAt(i);
            delete dlw->takeItem(i);
            devlist.erase(devlist.begin()+i);
            delete device;
            break;
        }
    }

    dumpDevices();
    updateErrorDisplay();
}

void Layouts::slotConnect(shared_ptr<IDevice> dev, bool settings_retained)
{
    Debug("Layouts::slotConnect(pid = 0x%x), connected = %d", dev->pid(), (int)dev->connected());

    globalsettings_tab->appendToLog(stringbuilder() << "device connected: " << dev->name());

    if (g_heavyConnect)
    {
        // forget features
        for (auto existing : devlist)
        {
            if (existing && existing->device_ == dev)
            {
                existing->device()->forget();
                existing->forget();
                existing->startup();
            }
        }
    }

    unsigned int r = dlw->currentRow();
    if (r < devlist.size() && devlist[r] && devlist[r]->device_ == dev)
    {
        devlist[r]->startup();
    }

    int i = 0;
    for (auto existing : devlist)
    {
        if (existing && existing->device_ == dev)
        {
            if (dlw->count() > i)
            {
                dlw->item(i)->setForeground(Qt::black);
                dlw->item(i)->setText(stringbuilder() << existing->getDeviceName());
                dlw->viewport()->update();
            }

            if (!settings_retained)
            {
                existing->settings_lost();
            }
        }

        i++;
    }

    onConfigChange(dev);
}

void Layouts::slotDisconnect(shared_ptr<IDevice> dev)
{
    Debug("Layouts::slotDisconnect(pid = 0x%x), connected = %d", dev->pid(), (int)dev->connected());

    globalsettings_tab->appendToLog(stringbuilder() << "device disconnected: " << dev->name());

    if (currow >= 0)
    {
        onDeviceSelectionChanged();
    }

    int i = 0;
    for (auto existing : devlist)
    {
        if (existing && existing->device_ == dev)
        {
            if (dlw->count() > i)
            {
                dlw->item(i)->setForeground(Qt::gray);
                dlw->viewport()->update();
            }
        }

        i++;
    }

    onConfigChange(dev);
}

// an error occurred, either global (dev=nullptr) or device specific (dev=device)
//
void Layouts::showerror(Device *dev)
{
    ErrorInfo errorInfo;
    stringbuilder str;

    if (dev)
    {
        str << "Error for device " << std::hex << dev->pid() << std::dec << ": ";
        errorInfo = dev->errinfo;
    }
    else
    {
        // store global error
        global_errinfo.errcode = devio::errcode;
        global_errinfo.errcode_sys = devio::errcode_sys;
        global_errinfo.errstr = devio::errcode_str ? devio::errcode_str : "";

        errorInfo = global_errinfo;
        str << "Error: ";
    }

    globalsettings_tab->appendToLog( str <<  describe_err(errorInfo.errcode,
                                                            errorInfo.errcode_sys,
                                                            errorInfo.errstr.c_str()));

    updateErrorDisplay();
}

// Update error display to show current error using this priority:
//
// 1) An error on the selected device, or if "All Devices" is selected then show the global error.   (show as black font)
// 2) Enumerate device and show the first error found                                                (show as grey font)
// 3) The global error                                                                               (show as grey font)
//
void Layouts::updateErrorDisplay()
{
    ErrorInfo err;
    err.errcode = ErrorCode::Success;
    bool selected = false;

    // First, look at selected device for an error
    unsigned int r = dlw->currentRow();
    if (r < devlist.size())
    {
        if (r == 0)
        {
            // first row is global error
            if (global_errinfo.errcode != ErrorCode::Success)
            {
                err = global_errinfo;
                selected = true;
            }
        }
        else if (devlist[r])
        {
            // subsequent rows are devices
            if (devlist[r]->errinfo.errcode != ErrorCode::Success)
            {
                err = devlist[r]->errinfo;
                selected = true;
            }
        }
    }

    // if no error on selected device, enumerate device list looking for an error
    if (err.errcode == ErrorCode::Success)
    {
        for (auto dev : devlist)
        {
            if (dev && dev->errinfo.errcode != ErrorCode::Success)
            {
                err = dev->errinfo;
                break;
            }
        }
    }

    // if no error on any device, look at global error
    if (err.errcode == ErrorCode::Success)
    {
        if (global_errinfo.errcode != ErrorCode::Success)
        {
            err = global_errinfo;
        }
    }

    if (err.errcode == ErrorCode::Success)
    {
        // no error
        status_label->setText("");
    }
    else
    {
        const char *color = selected ? "Black" : "Grey";

        status_label->setText(stringbuilder() << "<font color='" << color << "'>" << describe_err(err.errcode,
                                                              err.errcode_sys,
                                                              err.errstr.c_str()) << "</font>");
    }
}

void Layouts::onDeviceSelectionChanged()
{
    unsigned int r = dlw->currentRow();

    if (r < devlist.size())
    {
        feature_stack->setCurrentIndex(r);
        config_stack->setCurrentIndex(r);

        if (devlist[r])
        {
            devlist[r]->startup();
        }
    }

    updateErrorDisplay();
}

void Layouts::onCurrentDeviceChanged(QListWidgetItem * , QListWidgetItem * )
{
    auto items = dlw->selectedItems();
    if (items.size() == 0)
    {
        return;
    }

    onDeviceSelectionChanged();
}

void Layouts::slotDeviceReset()
{
    Info("Layouts::slotDeviceReset()");

    unsigned int row = dlw->currentRow();
    if (row < devlist.size())
    {
        if (devlist[row])
        {
            devlist[row]->device()->forget();
            devlist[row]->forget();
            devlist[row]->startup();
        }
    }
}

// to help with debugging
void Layouts::dumpDevices()
{
    Info("----------------------------");
    Info("Bazinga device dump:");

    for(int i = 0; i < dlw->count(); i++)
    {
        QString devName = dlw->item(i)->text();
        Info("- %s", (const char*)(devName.toLocal8Bit().data()));
    }
    Info("----------------------------");
}

void Layouts::contextualMenu(const QPoint& point)
{    
    (void) point;

    QMenu *menu = new QMenu;
 
    menu->addAction(QString("Reset"), this, SLOT(slotDeviceReset()));

    menu->addAction(QString("Unpair"), this, SLOT(slotDeviceReset()));
    menu->exec(QCursor::pos());
}

string describe_err(devio::ErrorCode errcode,
                    unsigned long errcode_sys,
                    const char* errstr)
{
    stringbuilder ss;
    ss << errcode;

    if (errcode_sys)
    {
        ss << " - err #0x" << std::hex << errcode_sys << std::dec;
    }

    if (errstr)
    {
    	ss << " - '" << std::string(errstr) << "'";
    }

    return ss;
}

string describe_err()
{
    return describe_err(devio::errcode,
                        devio::errcode_sys,
                        devio::errcode_str);
}

string describe_err( bool bresult)
{
    if (bresult)
    {
        return "true";
    }

    stringbuilder ret;
    ret << "false (" << describe_err() << ")";
    return ret;
}

