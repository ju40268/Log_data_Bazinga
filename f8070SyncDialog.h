#pragma once

#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QStackedLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QElapsedTimer>
#include <string>
#include <map>
#include "devio.h"
#include "globalsettings.h"
#include "device.h"    // for ErrorInfo
#include "f8070.h"
#include "f8070SyncModel.h"
#include <QTableView>

using devio::IDeviceManager;
using devio::IDevice;
using devio::Subscriber;

struct ParamsUsageHint
{
    IFeature8070ColorLEDEffects::EffectId id;
    QString params;
    QString label;
};

struct ZoneTimer
{
    QElapsedTimer msSinceLastReport;
    int syncCount;
};

class SyncWrapper :
    public QFrame,
    public Subscriber < devio::IFeature8070ColorLEDEffects::Report >
{
    Q_OBJECT
public:
    SyncWrapper(shared_ptr<IDevice> dev, int zoneCount);

    shared_ptr<IDevice> device(){
        return m_dev;
    };
    void onSyncReport(const vector<Byte>& v);

    void initialize(void){
        for (int i = 0; i < m_zoneTimers.count(); i++)
        {
            m_zoneTimers[i].syncCount = 0;
            m_zoneTimers[i].msSinceLastReport.restart();

        }
    }

    int syncCount(int zoneIndex)
    {
        return m_zoneTimers[zoneIndex].syncCount;
    }

    void synchronize(int zoneIndex, int time)
    {
        m_dev->feature<IFeature8070ColorLEDEffects>()->SynchronizeEffect(zoneIndex, time);
        m_zoneTimers[zoneIndex].syncCount = 0;
    }

    f8070SyncModel& model(void)
    {
        return m_model;
    }

    qint64 msSinceLastReport(int zoneIndex)
    {
        quint64 val = m_zoneTimers[zoneIndex].msSinceLastReport.elapsed();
        m_zoneTimers[zoneIndex].msSinceLastReport.restart();
        return val;
    }

signals:
    void syncReport(SyncWrapper*, int index, int tics);

private:
    shared_ptr<IDevice> m_dev;
    f8070SyncModel m_model;
    QVector<ZoneTimer> m_zoneTimers;
};

struct effectDef
{
    IFeature8070ColorLEDEffects::EffectId id;

};

class f8070SyncDialog :
    public QWidget,
    Subscriber < IDeviceManager::ArrivalRemoval >
{
    Q_OBJECT

public:
    f8070SyncDialog(QWidget *parent = 0);
    virtual void onDeviceRemoval(shared_ptr<IDevice> dev) override;
    virtual void onDeviceArrival(shared_ptr<IDevice> dev) override;

    void buildUI();
    void updateUI();

    void enumerateDevices();
    public slots:
    void onSyncReport(SyncWrapper*, int, int);
    void onEffectChanged();
    void onStartSync();
    void disableSync(bool disable);
    void setSyncPeriod(int value);
    void syncPeriodChanged(int tickValue);

private:

    QWidget* m_devList;
    QList<SyncWrapper*> m_wrappers;
    QComboBox* m_effect;
    QLineEdit* m_params;
    QLabel* m_usageHint;
    QLineEdit* m_minReportsBeforeSync;
    QLineEdit* m_minDriftForSync;
    QElapsedTimer m_time;
    QElapsedTimer m_effectTimer;
    int m_baseCounter;

    bool m_sync;

    quint32 m_syncEffectPeriod;
    QTimer* m_periodChangeTimer;
    QLabel* m_syncPeriodLabel;
};
