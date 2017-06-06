#pragma once

#include <QTextEdit>
#include "feature.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSignalMapper>
#include <QCheckBox>
#include <QTimer>
#include <QTemporaryFile>
#include <vector>
#include "devio.h"
#include "Source/UnifyingBus.h"
#include "QProgressBar"

using devio::IFeature;
using devio::IFeature8111LatencyMonitoring;
using devio::Byte;
using devio::Subscriber;
using devio::UnifyingBus;
using std::vector;


class f8111 : public QWidget,
              public Subscriber<devio::IFeature8111LatencyMonitoring::Report>
{
    Q_OBJECT

    struct QualityLevel
    {
        QualityLevel(void)
        {
            indicator = NULL;
            thresholdEdit = NULL;
            runningCountLabel = NULL;
            runningCount = 0;
        }

        QProgressBar* indicator;
        QLineEdit* thresholdEdit;
        QLabel* runningCountLabel;
        quint32 runningCount;
    };

public:
    explicit f8111(shared_ptr<devio::IFeature8111LatencyMonitoring> f, Feature* base);
    
    virtual void onLatencyEvent(const IFeature8111LatencyMonitoring::LatencyEventInfo& info);

protected:
    void createUI(void);
    void notifyText(const QString &text, bool insertCRLF = false);
    
protected slots:
    void dumpCapabilities(void);
    void applyBoundaries(void);
    void onNotificationContextMenu(const QPoint &pos);
    void onClearNotifications(void);
    void onClearLatencyCounts(void);
    void queryRSSI(void);
    void onEnableLogFile(bool enabled);

private:
    float linkQuality(const IFeature8111LatencyMonitoring::LatencyEventInfo& info);
    
private:
    shared_ptr<devio::IFeature8111LatencyMonitoring> feature8111;
    Feature* baseFeature;
    QTextEdit* notifications;
    shared_ptr<UnifyingBus> m_bus;
    quint16 m_lastRSSI;
    QLabel* m_lastRSSILabel;
    QFile* m_logFile;

    // Boundaries
    QCheckBox* notificationsEnabledCheckbox;
    QLineEdit* notificationPeriodInSeconds;
    std::vector<QLineEdit*> boundaryEdits;
    std::vector<QualityLevel> levels;
};
