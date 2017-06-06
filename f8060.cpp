#include "feature.h"
#include "f8060.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include <QMenu>
#include <QGroupBox>
#include <QValidator>
#include <QPushButton>

using devio::Delivery;
using devio::CalibrationData;

f8060::f8060(shared_ptr<devio::IFeature8060ReportRate> f, Feature* base, shared_ptr<IDevice> device):
    QWidget(),
    feature8060(f),
    baseFeature(base),
    _device(device),
    notifications(NULL),
    curReportRate(2)
{
    createUI();
    refreshData();
    refreshUI();

#ifdef Q_OS_WIN
    m_inputTrackerDlg = NULL;
#endif
}


void f8060::createUI(void)
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    setLayout(vLayout);

    {
        QGroupBox* group = new QGroupBox("Select a new report rate");
        QVBoxLayout* dataLayout = new QVBoxLayout;
        group->setLayout(dataLayout);
        
        uiReportRates = new QComboBox;
        connect(uiReportRates, SIGNAL(currentIndexChanged(int)), this, SLOT(onReportRateChanged(int)));
        dataLayout->addWidget(uiReportRates);
        
        vLayout->addWidget(group);
    }

    {
        QPushButton *getReportRateBtn = new QPushButton("Get Report Rate");
        connect(getReportRateBtn, SIGNAL(clicked()), this, SLOT(onGetReportRate()));
        
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(getReportRateBtn);
        vLayout->addLayout(hLayout);
    }

    // Test Report Rate
    {
#ifdef Q_OS_WIN
        QPushButton *testReportRateBtn = new QPushButton("Test Report Rate");
        connect(testReportRateBtn, SIGNAL(clicked()), this, SLOT(onRawInputDlg()));

        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(testReportRateBtn);
        vLayout->addLayout(hLayout);
#endif
    }

    // Notifications
    {
        notifications = new QTextEdit;
        notifications->setReadOnly(true);
        notifications->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(notifications, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onNotificationContextMenu(const QPoint&)));
        vLayout->addWidget(notifications);
    }
}

void f8060::notifyText(const QString &text, bool insertCRLF)
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

void f8060::onNotificationContextMenu(const QPoint &pos)
{
    QMenu popup(this);
    popup.addAction("Clear", this, SLOT(onClearNotifications()));
    popup.exec(notifications->mapToGlobal(pos));
}

void f8060::onClearNotifications(void)
{
    notifications->clear();
}

void f8060::onReportRateChanged(int index)
{
    int rate = uiReportRates->itemData(index).toInt();
    if (!feature8060->SetReportRate(rate))
    {
        notifyText(QString("ERROR: SetReportRate failed (%1ms) (Make sure your device is in host mode)").arg(rate));
    }
    else
    {
        curReportRate = rate;
        notifyText(QString("Report rate is now set to %1ms").arg(rate));
    }
}

void f8060::onGetReportRate(void)
{
    uint8_t rate{ 0 };
    if (feature8060->GetReportRate(&rate))
    {
        notifyText(QString("Current report rate is %1ms").arg(rate));
    }
    else
    {
        notifyText("ERROR: GetReportRate failed");
    }
}

#ifdef Q_OS_WIN

void f8060::onRawInputDlg(void)
{
    if (NULL == m_inputTrackerDlg)
    {
        QString pid = QString::number(baseFeature->device->pid(), 16);
        QString name = QString::fromStdString(baseFeature->device->getDeviceName());
        name = name.mid(name.indexOf("- ")).remove("- ");

        m_inputTrackerDlg = new cInputTracker(pid, name, curReportRate);
        connect(m_inputTrackerDlg, SIGNAL(destroyed(QObject*)), this, SLOT(onInputTrackerDlgDestroyed(QObject*)));
    }

    m_inputTrackerDlg->show();
}

void f8060::onInputTrackerDlgDestroyed(QObject*)
{
    if (NULL != m_inputTrackerDlg)
    {
        m_inputTrackerDlg = NULL;
    }
}

#endif

void f8060::refreshData(void)
{
    // Current rate
    curReportRate = 4; // 4ms
    if (!feature8060->GetReportRate(&curReportRate))
    {
        notifyText("ERROR: GetReportRate failed");
    }

    // Supported rates
    uint8_t supportedReportRates = 0;
    if (!feature8060->GetReportRateList(&supportedReportRates))
    {
        notifyText("ERROR: GetReportRates failed");
    }
    else
    {
        reportRates = supportedReportRates;
    }
}

void f8060::refreshUI(void)
{
    uiReportRates->blockSignals(true);
    // Clear the list
    uiReportRates->clear();
    for (int i = 0; i < 8; i++)
    {
        if (reportRates.bit(i))
        {
            // The user data contains the actual report rate
            uiReportRates->addItem(QString("%1ms").arg(i+1), i+1);
        }
    }
    
    // Select the current one
    int uiCount = uiReportRates->count();
    for (int i = 0; i < uiCount; i++)
    {
        if (uiReportRates->itemData(i).toInt() == curReportRate)
        {
            uiReportRates->setCurrentIndex(i);
        }
    }
    uiReportRates->blockSignals(false);
}




