#include "f8070SyncDialog.h"
#include <QLayout>
#include <QThread>
#include <QTime>
#include <QDebug>
#include <QHeaderView>

using devio::cdevio;

const ParamsUsageHint _effectHints[] = {
    { IFeature8070ColorLEDEffects::EF_OFF ,         "",              "None"},
    { IFeature8070ColorLEDEffects::EF_FIXED, "11DD FF02 0000" , "(Red), (Green), (Blue), Ramp (0-2) " },
    { IFeature8070ColorLEDEffects::EF_PULSE, "4400 ff50 0000" ,  "(Red), (Green), (Blue), Delay (ms)"  },
    { IFeature8070ColorLEDEffects::EF_CYCLE,      "0000 0000 0009 0000" ,  " 0, 0, 0, 0, 0, Speed (ms) MSB, Speed (ms) LSB, Intensity (1-100)" },
    { IFeature8070ColorLEDEffects::EF_COLOR_WAVE, "0000 0000 0000 8800 0013" ,  "0,0,0,0,0,0, Speed (ms) LSB, Direction (1-3),  Intensity (1-100), Speed (ms) MSB" },
    { IFeature8070ColorLEDEffects::EF_STARLIGHT, "0000 0000 0000" ,  "Not supported" },
    { IFeature8070ColorLEDEffects::EF_LIGHT_ON_PRESS, "0", "Not supported"  },
    { IFeature8070ColorLEDEffects::EF_AUDIO, "" ,"not supported"},
    { IFeature8070ColorLEDEffects::EF_BOOT_UP, ""," not supported" },
    { IFeature8070ColorLEDEffects::EF_DEMO, "" , "not supported" },
    { IFeature8070ColorLEDEffects::EF_PULSE_WAVEFORM, "00ff 000f a000", "(Red), (Green), (Blue), Speed(ms) MSB, Speed(ms) LSB, Waveform(1 - 5), Intensity(1 - 100) " }
};

f8070SyncDialog::f8070SyncDialog(QWidget* parent) :
QWidget(parent)

{
    m_sync = false;
    m_syncEffectPeriod = 0;
    m_sync = true;
    m_periodChangeTimer = new QTimer(this);
    m_periodChangeTimer->setInterval(500);
    m_periodChangeTimer->setSingleShot(true);
    if (!Subscriber<ArrivalRemoval>::subscribe(cdevio,
        devio::Delivery::RunLoop,
        "f8070SyncDialog::ArrivalRemoval"))
    {
    }

    enumerateDevices();
    buildUI();
    updateUI();
    onStartSync();
}



void f8070SyncDialog::onDeviceRemoval(shared_ptr<IDevice> dev)
{
    enumerateDevices();
    updateUI();
}
void f8070SyncDialog::onDeviceArrival(shared_ptr<IDevice> dev)
{
    enumerateDevices();
    updateUI();
    onStartSync();
}

void f8070SyncDialog::onEffectChanged()
{
    //m_params->setText(_effectHints[m_effect->currentIndex()].params);
    //m_usageHint->setText(QString("Params: %1").arg(_effectHints[m_effect->currentIndex()].label));
}

void f8070SyncDialog::buildUI()
{

    setLayout(new QVBoxLayout());
    m_devList = new QWidget();

//    m_usageHint = new QLabel("Params: ");
//    layout()->addWidget(m_usageHint);
    m_effect = new QComboBox();
///    m_params = new QLineEdit();
//    layout()->addWidget(m_params);

    for (int i = 0; i <= IFeature8070ColorLEDEffects::EF_PULSE_WAVEFORM; i++)
    {
        IFeature8070ColorLEDEffects::EffectId e = ((IFeature8070ColorLEDEffects::EffectId) i);
        m_effect->addItem(f8070::toString(e), e);
    }
    layout()->addWidget(new QLabel("Effect"));
    layout()->addWidget(m_effect);

    // select pulse by default
    m_effect->setCurrentIndex(m_effect->count()-1);

    connect(m_effect, SIGNAL(currentIndexChanged(int)), this, SLOT(onEffectChanged()));

    layout()->addItem(new QSpacerItem(0, 30));

    // Add a simple slider for the speed
    layout()->addWidget(new QLabel("Period"));
    QSlider* periodSlider = new QSlider;
    periodSlider->setRange(1, 20);
    periodSlider->setTickInterval(20);
    periodSlider->setPageStep(1);
    periodSlider->setOrientation(Qt::Horizontal);
    periodSlider->setTracking(false);
    connect(periodSlider, SIGNAL(valueChanged(int)), this, SLOT(syncPeriodChanged(int)));
    layout()->addWidget(periodSlider);
    m_syncPeriodLabel = new QLabel;
    layout()->addWidget(m_syncPeriodLabel);

    layout()->addItem(new QSpacerItem(0, 30));

    QPushButton* dosync = new QPushButton("Sync");
    connect(dosync, SIGNAL(pressed()), this, SLOT(onStartSync()));

    QCheckBox* disableSync = new QCheckBox("Disable Sync");
    connect(disableSync, SIGNAL(toggled(bool)), this, SLOT(disableSync(bool)));

    layout()->addWidget(dosync);
    layout()->addWidget(disableSync);

    layout()->addItem(new QSpacerItem(0, 30));

    // Threshold: MinReports
    {
        QHBoxLayout* hbox = new QHBoxLayout;
        QLabel* l = new QLabel("Minimum reports between sync requests: ");
        m_minReportsBeforeSync = new QLineEdit;
        m_minReportsBeforeSync->setMaximumWidth(50);
        m_minReportsBeforeSync->setValidator(new QIntValidator(0, 1000));
        m_minReportsBeforeSync->setText("3");
        hbox->addWidget(l);
        hbox->addWidget(m_minReportsBeforeSync);
        hbox->addStretch();
        ((QBoxLayout*)layout())->addLayout(hbox);
    }

    // Threshold: Drift
    {
        QHBoxLayout* hbox = new QHBoxLayout;
        QLabel* l = new QLabel("Minimum host drift allowed (%): ");
        m_minDriftForSync = new QLineEdit;
        m_minDriftForSync->setValidator(new QIntValidator(0, 100));
        m_minDriftForSync->setMaximumWidth(50);
        m_minDriftForSync->setText("3");
        hbox->addWidget(l);
        hbox->addWidget(m_minDriftForSync);
        hbox->addStretch();
        ((QBoxLayout*)layout())->addLayout(hbox);
    }

    periodSlider->setValue(5);

    setMinimumWidth(800);
}

void f8070SyncDialog::updateUI()
{
    //Re'draw' all the wrappers
    delete (m_devList);

    m_devList = new QWidget();
    m_devList->setLayout(new QVBoxLayout());
    for (int i = 0; i < m_wrappers.count(); i++)
    {
        SyncWrapper* s = m_wrappers[i];
        m_devList->layout()->addWidget(s);

        connect(s, SIGNAL(syncReport(SyncWrapper*, int, int)), this, SLOT(onSyncReport(SyncWrapper*, int, int)));
    }
    layout()->addWidget(m_devList);
}

void f8070SyncDialog::onStartSync()
{
    IFeature8070ColorLEDEffects::EffectId effect = (IFeature8070ColorLEDEffects::EffectId) m_effect->currentData().toInt();

    vector<Byte> params;
    //Convert text to bytes
    QString paramBucket = _effectHints[m_effect->currentIndex()].params;
    paramBucket = paramBucket.remove("0x");
    paramBucket = paramBucket.remove("#");
    paramBucket = paramBucket.remove(" ");
    for (int i = 0; i < paramBucket.count() - 1; i+=2)
    {
        params.push_back( paramBucket.mid(i,2).toInt(NULL, 16));
    }

    // Set the sync period
    switch(effect)
    {
        case IFeature8070ColorLEDEffects::EF_PULSE_WAVEFORM:
            params[3] = ((quint16)m_syncEffectPeriod & 0xff00) >> 8;
            params[4] = ((quint16)m_syncEffectPeriod & 0xff);
            break;
        case IFeature8070ColorLEDEffects::EF_CYCLE :
            params[5] = ((quint16)m_syncEffectPeriod & 0xff00) >> 8;
            params[6] = ((quint16)m_syncEffectPeriod & 0xff);
            break;

        case IFeature8070ColorLEDEffects::EF_COLOR_WAVE:
            params[9] = ((quint16)m_syncEffectPeriod & 0xff00) >> 8;
            params[6] = ((quint16)m_syncEffectPeriod & 0xff);
            break;
        default:
            break;
    }

    //Tell each     to start an effect and send SW control
    for (int i = 0; i < m_wrappers.count(); i++)
    {
        auto f = m_wrappers[i]->device()->feature<IFeature8070ColorLEDEffects>();

        f->SetSWControl(true, true);

        int zoneCount=0;
        IFeature8070ColorLEDEffects::NvCapability nvCapability;
        IFeature8070ColorLEDEffects::ExtCapability extCapability;
        f->GetInfo(zoneCount, nvCapability, extCapability);
        bool success = true;

        // Do primary and logo zones
        success &= f->SetZoneEffect(IFeature8070ColorLEDEffects::ZONE_PRIMARY, effect, params);
        success &= f->SetZoneEffect(IFeature8070ColorLEDEffects::ZONE_LOGO, effect, params);

        m_wrappers[i]->initialize();
        m_wrappers[i]->model().removeRows(0, m_wrappers[i]->model().rowCount());
    }

    // (Re)Start our effect timer now
    m_effectTimer.start();
}


//*************************************************************************
//
// f8070SyncDialog::disableSync
//
//*************************************************************************

void f8070SyncDialog::disableSync(bool disable)
{
    m_sync = !disable;
}


//*************************************************************************
//
// f8070SyncDialog::setSyncPeriod
//
//*************************************************************************

void f8070SyncDialog::setSyncPeriod(int value)
{
    m_syncPeriodLabel->setText(QString("%1 ms").arg(value));
    m_syncEffectPeriod = value;
}


//*************************************************************************
//
// f8070SyncDialog::syncPeriodChanged
//
//*************************************************************************

void f8070SyncDialog::syncPeriodChanged(int tickValue)
{
    setSyncPeriod(1000 * tickValue);
}


void f8070SyncDialog::enumerateDevices()
{
    m_wrappers.clear();

    vector<shared_ptr<IDevice>> devices = cdevio.devices();

    for (size_t i = 0; i < devices.size(); i++)
    {
        shared_ptr<IFeature8070ColorLEDEffects> f = devices[i]->feature<IFeature8070ColorLEDEffects>();
        if (f == nullptr)
        {
            continue;
        }
        m_wrappers.push_back(new SyncWrapper(devices[i], 3));
    }
}


// not sync to LGS, only sync slave to master
void f8070SyncDialog::onSyncReport(SyncWrapper* sw, int zoneIndex, int time)
{
    if (0 == m_syncEffectPeriod)
    {
        return;
    }

    int hostEffectTime = (m_effectTimer.elapsed() % m_syncEffectPeriod);
    int diffWithLocalTime = hostEffectTime-time;
    bool adjusted = 0;
    float hostDriftPercentage = 100.0f * abs(diffWithLocalTime)/(float)m_syncEffectPeriod;
    int msSinceLastReport = sw->msSinceLastReport(zoneIndex);

    // If the diff is very large ( > 1000 ) force it
    // If the diff is very small ( < 100 ) ignore it
    int minReports = m_minReportsBeforeSync->text().toInt();
    int driftPerentage = m_minDriftForSync->text().toInt();

    bool doScheduledSynchronize = (minReports > 0) ? (0 == (sw->syncCount(zoneIndex) % minReports)) : true;
    doScheduledSynchronize = doScheduledSynchronize && (hostDriftPercentage > driftPerentage);

    bool doForcedSynchronize = false;//(abs(diffWithLocalTime) > 1000);

    if (m_sync)
    {
        quint32 syncEffectHidTime = 0;
        if (doScheduledSynchronize || doForcedSynchronize)
        {
            QElapsedTimer syncEffectTime;
            syncEffectTime.start();
            sw->device()->feature<IFeature8070ColorLEDEffects>()->SynchronizeEffect(zoneIndex, diffWithLocalTime);
            adjusted = 1;
            syncEffectHidTime = syncEffectTime.elapsed();
        }

        QString s = QString("%1: pid=0x%2, zoneIndex=%3, deviceEffectTime=%4 hostEffectTime=%5 diffWithHost=%6, timeSinceLastEvent=%7 adjusted=%8 syncCount=%9 syncEffectHidTime=%10ms")
            .arg((quint32)QTime::currentTime().msecsSinceStartOfDay())
            .arg(sw->device()->pid(), 0, 16)
            .arg(zoneIndex)
            .arg(time)
            .arg(hostEffectTime)
            .arg(diffWithLocalTime)
            .arg(msSinceLastReport)
            .arg(adjusted)
            .arg(sw->syncCount(zoneIndex))
            .arg(adjusted ? syncEffectHidTime : 0);
        qDebug() << s << endl;
    }

    sw->model().addSyncEvent(zoneIndex, time, diffWithLocalTime, hostDriftPercentage, msSinceLastReport, adjusted);
}



SyncWrapper::SyncWrapper(shared_ptr<IDevice> dev, int zoneCount) : m_dev(dev)
{
    m_zoneTimers.resize(zoneCount);

    subscribe(m_dev->feature<IFeature8070ColorLEDEffects>());

    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    //setFixedSize(350, 130);
    m_dev = dev;

    QLabel* devName = new QLabel(QString(dev->name().c_str()),this);

    setLayout(new QVBoxLayout());

    QWidget* nameBlock = new QWidget();
    nameBlock->setLayout(new QHBoxLayout());
    nameBlock->layout()->addWidget(devName);
    layout()->addWidget(nameBlock);

    // List view to log sync event data
    QTableView* eventView = new QTableView;
    eventView->setModel(&m_model);
    eventView->verticalHeader()->setVisible(false);
    eventView->resizeColumnsToContents();
    eventView->setGridStyle(Qt::NoPen);
    eventView->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
    eventView->verticalHeader()->setDefaultSectionSize(18);
    connect(&m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), eventView, SLOT(scrollToBottom()) );
    layout()->addWidget(eventView);
}


void SyncWrapper::onSyncReport(const vector<Byte>& v)
{
    int reportZoneIndex = v[0];
    int reporttime = v[1].msb_word();
    m_zoneTimers[reportZoneIndex].syncCount++;
    emit syncReport(this, reportZoneIndex, reporttime);
}
