#include "feature.h"
#include "f8111.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include <QMenu>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QTextStream>

using devio::Delivery;

// Persistence
#define F8111_LOG_TO_FILE                                       "f8111_LogToFile"

f8111::f8111(shared_ptr<devio::IFeature8111LatencyMonitoring> f, Feature* base) :
    QWidget(),
    feature8111(f),
    baseFeature(base),
    notifications(NULL),
    m_lastRSSI(0),
    m_lastRSSILabel(new QLabel),
    m_logFile(NULL)
{
    auto receiver = dynamic_pointer_cast<devio::UnifyingBus>(base->device->device()->receiver());

    if (receiver)
    {
        m_bus = receiver;
        queryRSSI();
    }

    createUI();
    
    subscribe(feature8111,Delivery::Immediate);

    onEnableLogFile(true);
}


void f8111::createUI(void)
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    setLayout(vLayout);
    
    QVBoxLayout* vButtonLayout = new QVBoxLayout;
    vLayout->addLayout(vButtonLayout);
    
    // GetCapabilities
    {
        QGroupBox* group = new QGroupBox("Capabilities");
        group->setLayout(new QHBoxLayout);
        QPushButton* getCapabilities = new QPushButton("GetCapabilities");
        connect(getCapabilities, SIGNAL(pressed()), this, SLOT(dumpCapabilities()));
        group->layout()->addWidget(getCapabilities);
        vButtonLayout->addWidget(group);
        vButtonLayout->addSpacerItem(new QSpacerItem(0, 0));
    }

    // Notifications
    {
        const int defaultBoundaries[6] = { 1, 2, 5, 10, 20, 100 };
        QGroupBox* group = new QGroupBox("Notifications");
        QVBoxLayout* groupLayout = new QVBoxLayout;
        group->setLayout(groupLayout);
        notificationsEnabledCheckbox = new QCheckBox("Enable");
        notificationsEnabledCheckbox->setChecked(true);
        group->layout()->addWidget(notificationsEnabledCheckbox);

        QWidget* notificationPeriod = new QWidget;
        notificationPeriod->setLayout(new QHBoxLayout);
        notificationPeriod->layout()->setContentsMargins(0, 0, 0, 0);
        notificationPeriod->layout()->addWidget(new QLabel("Period (seconds):"));
        notificationPeriodInSeconds = new QLineEdit;
        notificationPeriodInSeconds->setValidator(new QIntValidator(0, 10000));
        notificationPeriodInSeconds->setMaximumWidth(50);
        notificationPeriodInSeconds->setText("8");
        notificationPeriod->layout()->addWidget(notificationPeriodInSeconds);
        ((QHBoxLayout*)notificationPeriod->layout())->addStretch();
        group->layout()->addWidget(notificationPeriod);

        // Boundaries
        QHBoxLayout* hbox = new QHBoxLayout;
        QLabel* boundariesLabel = new QLabel("Boundaries (ms): ");
        hbox->addWidget(boundariesLabel);
        for (int i = 0; i < 6; i++)
        {
            QLineEdit* boundary = new QLineEdit;
            boundary->setText(QString("%1").arg(defaultBoundaries[i]));
            boundary->setValidator(new QIntValidator(1, 255));
            boundaryEdits.push_back(boundary);
            hbox->addWidget(boundary);
        }
        QPushButton* apply = new QPushButton("Apply");
        connect(apply, SIGNAL(pressed()), this, SLOT(applyBoundaries()));
        hbox->addWidget(apply);
        groupLayout->addLayout(hbox);

        vButtonLayout->addWidget(group);
        vButtonLayout->addSpacerItem(new QSpacerItem(0, 0));
    }

    // Notifications
    {
        notifications = new QTextEdit;
        notifications->setReadOnly(true);
        notifications->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(notifications, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onNotificationContextMenu(const QPoint&)));
        vLayout->addWidget(notifications);
    }

    // Wireless Quality
    // Show 4 standard bars
    // Nothing fancy here, just 4 progress bars
    {
        QGroupBox* wirelessQuality = new QGroupBox("Quality Monitoring");
        QVBoxLayout* qualityLayout = new QVBoxLayout;
        wirelessQuality->setLayout(qualityLayout);
        
        QGridLayout* grid = new QGridLayout;
        for (int i = 0; i < 4; i++)
        {
            QualityLevel ql;
            ql.thresholdEdit = new QLineEdit;
            ql.thresholdEdit->setFixedWidth(50);
            ql.indicator = new QProgressBar;
            ql.indicator->setValue(100);
            ql.indicator->setFixedHeight((i + 1) * 20);
            ql.runningCountLabel = new QLabel("0");
            ql.runningCountLabel->setFixedWidth(50);
            ql.indicator->setOrientation(Qt::Vertical);
            grid->addWidget(ql.indicator, 0, i + 1, Qt::AlignHCenter | Qt::AlignBottom);
            grid->addWidget(ql.thresholdEdit, 1, i + 1);
            grid->addWidget(ql.runningCountLabel, 2, i + 1);
            levels.push_back(ql);
        }

        levels[0].thresholdEdit->setText("0");
        levels[1].thresholdEdit->setText("94");
        levels[2].thresholdEdit->setText("96");
        levels[3].thresholdEdit->setText("98");

        // Spacers
        grid->addItem(new QSpacerItem(0, 0), 0, 5);
        grid->addItem(new QSpacerItem(0, 0), 1, 5);
        grid->addItem(new QSpacerItem(0, 0), 2, 5);
        grid->setColumnStretch(5, 1);

        grid->addWidget(new QLabel("Threshold (%)"), 1, 0);
        grid->addWidget(new QLabel("Count"), 2, 0);

        qualityLayout->addLayout(grid);

        QPushButton* resetCounts = new QPushButton("Reset Counts");
        connect(resetCounts, SIGNAL(clicked()), this, SLOT(onClearLatencyCounts()));
        qualityLayout->addWidget(resetCounts);
        qualityLayout->addWidget(m_lastRSSILabel);
        
        QCheckBox* writeToLog = new QCheckBox("Log Events ('%localappdata%/Bazinga/')");
        writeToLog->setChecked(true);
        connect(writeToLog, SIGNAL(toggled(bool)), this, SLOT(onEnableLogFile(bool)));
        qualityLayout->addWidget(writeToLog);

        vLayout->addWidget(wirelessQuality);
    }

    // Poll for signal strength
    if (m_bus)
    {
        QTimer* rssiTimer = new QTimer(this);
        connect(rssiTimer, &QTimer::timeout, this, &f8111::queryRSSI);
        rssiTimer->setInterval(2000);
        rssiTimer->start();
    }
}

void f8111::onLatencyEvent(const IFeature8111LatencyMonitoring::LatencyEventInfo& info)
{
    QString qualityStr = QString("%1").arg(linkQuality(info), 0, 'f', 2);
    QString latencyEvent = QString("onLatencyEvent: total=%1, boundaries={%2, %3, %4, %5, %6, %7}, over=%8, quality=%9%").
        arg(info.totalMessageCount).
        arg(info.messageCountBoundary1).
        arg(info.messageCountBoundary2).
        arg(info.messageCountBoundary3).
        arg(info.messageCountBoundary4).
        arg(info.messageCountBoundary5).
        arg(info.messageCountBoundary6).
        arg(info.messageCountOverflow).
        arg(qualityStr);

    // Convert to a percentage and update our monitoring
    float perc = linkQuality(info);
    for (int i = 3; i >= 0; i--)
    {
        if (perc >= levels[i].thresholdEdit->text().toFloat())
        {
            levels[i].runningCount++;
            levels[i].runningCountLabel->setText(QString("%1").arg(levels[i].runningCount));
            break;
        }
    }

    notifyText(latencyEvent);

    if (m_logFile)
    {
        QTextStream ts(m_logFile);
        ts << QDateTime::currentMSecsSinceEpoch() << "," <<
            info.totalMessageCount << "," <<
            info.messageCountBoundary1 << "," <<
            info.messageCountBoundary2 << "," <<
            info.messageCountBoundary3 << "," <<
            info.messageCountBoundary4 << "," <<
            info.messageCountBoundary5 << "," <<
            info.messageCountBoundary6 << "," <<
            info.messageCountOverflow << "," <<
            qualityStr << "," <<
            m_lastRSSI << endl;
    }
}

void f8111::dumpCapabilities(void)
{
    IFeature8111LatencyMonitoring::Capabilities caps;
    if (!feature8111->GetCapabilities(caps))
    {
        notifyText("ERROR: feature8111->GetCapabilities");
        return;
    }

    notifyText(QString("Min Period: %1s").arg(caps.minPeriodInSecs));
    notifyText(QString("Max Period: %1s").arg(caps.maxPeriodInSecs));
    notifyText(QString("Flags: 0x%1").arg(caps.flags, 0, 16));
}

void f8111::applyBoundaries(void)
{
    // Collect our  buckets
    std::vector<unsigned int> boundaries;
    for (size_t i = 0; i < boundaryEdits.size(); i++)
    {
        boundaries.push_back(boundaryEdits[i]->text().toUInt());
    }

    unsigned int periodInSeconds = notificationPeriodInSeconds->text().toUInt();

    if (!feature8111->EnableNotifications(notificationsEnabledCheckbox->isChecked(), periodInSeconds, boundaries))
    {
        notifyText("ERROR: feature8111->EnableNotifications");
        return;
    }
}

void f8111::notifyText(const QString &text, bool insertCRLF)
{
    // Jump to the end (to restore the cursor)
    QTextCursor tc = notifications->textCursor();
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    notifications->setTextCursor(tc);
    
    notifications->insertPlainText(text + QString("\n"));
    if (insertCRLF)
    {
        notifications->append("\n");
    }
    
    // Jump to the end (to scroll to the end)
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    notifications->setTextCursor(tc);
}

void f8111::onNotificationContextMenu(const QPoint &pos)
{
    QMenu popup(this);
    popup.addAction("Clear", this, SLOT(onClearNotifications()));
    popup.exec(notifications->mapToGlobal(pos));
}

void f8111::onClearNotifications(void)
{
    notifications->clear();
}


//*************************************************************************
//
// f8111::onClearLatencyCounts
//
//*************************************************************************

void f8111::onClearLatencyCounts(void)
{
    for (int i = 0; i < 4; i++)
    {
        levels[i].runningCount = 0;
        levels[i].runningCountLabel->setText("0");
    }
}


//*************************************************************************
//
// f8111::linkQuality
//
//*************************************************************************

float f8111::linkQuality(const IFeature8111LatencyMonitoring::LatencyEventInfo& info)
{
    // Link Quality in % = 100 x N1 / ((1xN1)+( 2 x N2)+ (5 x N5)+ (10 x N10))
    // Where boundaries are 1ms, 2ms, 5ms, 10ms, etc
    if (0 == info.messageCountBoundary1)
    {
        return 0.0f;
    }

    float lq_top = (100.0f * info.messageCountBoundary1);
    float lq_bottom = 0.0f;
    for (int i = 0; i < 6; i++)
    {
        switch (i)
        {
        case 0:
            lq_bottom += (boundaryEdits[0]->text().toUInt() * info.messageCountBoundary1);
            break;
        case 1:
            lq_bottom += (boundaryEdits[1]->text().toUInt() * info.messageCountBoundary2);
            break;
        case 2:
            lq_bottom += (boundaryEdits[2]->text().toUInt() * info.messageCountBoundary3);
            break;
        case 3:
            lq_bottom += (boundaryEdits[3]->text().toUInt() * info.messageCountBoundary4);
            break;
        case 4:
            lq_bottom += (boundaryEdits[4]->text().toUInt() * info.messageCountBoundary5);
            break;
        case 5:
            lq_bottom += (boundaryEdits[5]->text().toUInt() * info.messageCountBoundary6);
            break;
        default:
            break;
        }
    }

    // Just return 100% for strange conditions
    if ((0 == lq_top) || (0 == lq_bottom))
    {
        return 100.0f;
    }

    return (lq_top / lq_bottom);
}


void f8111::queryRSSI(void)
{
    vector<Byte> query{ 0x10, 0xff, 0x81, 0xB4, 0x01, 0x00, 0x00 };
    vector<Byte> response;
    float str = 0;
    float res = 0;
    bool ok = m_bus->sendSynchronousCommand(query, response, devio::Cached::No);
    
    if (ok)
    {
        
        m_lastRSSI = response[6];
        m_lastRSSILabel->setText(QString("Last RSSI: %1").arg(m_lastRSSI));
    }
}


//*************************************************************************
//
// f8111::onEnableLogFile
//
//*************************************************************************

void f8111::onEnableLogFile(bool enabled)
{
    // Close existing file
    if (m_logFile)
    {
        m_logFile->close();
        delete m_logFile;
    }

    if (enabled)
    {
        // Generate a new file
        QString newFile(QString("Bazinga_f8111_latency_%1.txt").arg(QDateTime::currentMSecsSinceEpoch()));
        QDir bazingaDataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
        bazingaDataDir.mkpath(".");
        QString logFilePath = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).absoluteFilePath(newFile);
        m_logFile = new QFile(logFilePath);
        if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            notifyText(QString("Failed to create log file %1").arg(logFilePath));
            return;
        }

        // Write the header
        QTextStream ts(m_logFile);
        ts << "Timestamp (ms)," << "Total," << "1ms," << "2ms," << "5ms," << "10ms," << "20ms," << "100ms," ">100ms," << "Quality (Weighted %)," << "RSSI," << endl;
    }
}
