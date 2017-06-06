#include "feature.h"
#include "f8110.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include <QMenu>
#include <QGroupBox>

using devio::Delivery;

f8110::f8110(shared_ptr<devio::IFeature8110MouseButtonSpy> f, Feature* base) :
    QWidget(),
    feature8110(f),
    baseFeature(base),
    buttonCount(0),
    notifications(NULL),
    isSpying(false),
    uiButtonCount(NULL),
    uiSpyToggle(NULL),
    buttonState(0),
    buttonUsages(nullptr),
    buttonMappedUsages(nullptr)

{
    createUI();
    refreshData();
    refreshUI();
    
    subscribe(feature8110,Delivery::Immediate);
}


void f8110::createUI(void)
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    setLayout(vLayout);
    
    QVBoxLayout* vButtonLayout = new QVBoxLayout;
    vLayout->addLayout(vButtonLayout);
    
    // Button Count
    {
        QHBoxLayout* buttonCountLayout = new QHBoxLayout;
        QLabel* label = new QLabel("Button Count:");
        buttonCountLayout->addWidget(label);
        uiButtonCount = new QLabel;
        buttonCountLayout->addWidget(uiButtonCount);
        buttonCountLayout->addSpacerItem(new QSpacerItem(0,0));
        buttonCountLayout->setStretch(2, 1);
        vButtonLayout->addLayout(buttonCountLayout);
    }
    
    // Spy Start/Stop
    {
        QGroupBox* group = new QGroupBox("Button Events");
        group->setLayout(new QHBoxLayout);
        uiSpyToggle = new QPushButton("Start Spying");
        connect(uiSpyToggle, SIGNAL(pressed()), this, SLOT(onToggleSpy()));
        group->layout()->addWidget(uiSpyToggle);
        vButtonLayout->addWidget(group);
        vButtonLayout->addSpacerItem(new QSpacerItem(0,0));
    }

    // Usages
    {
        QGroupBox* group = new QGroupBox("Button Remapping");
        QHBoxLayout* buttonMapLayout = new QHBoxLayout;
        group->setLayout(buttonMapLayout);
    
        buttonUsages = new QComboBox();
        buttonMappedUsages = new QComboBox();
        group->layout()->addWidget(buttonUsages);
        group->layout()->addWidget(new QLabel("is mapped to"));
        group->layout()->addWidget(buttonMappedUsages);
        QPushButton* getRemapping = new QPushButton("Refresh");
        group->layout()->addWidget(getRemapping);
        QPushButton* resetUsages = new QPushButton("Restore Defaults");
        group->layout()->addWidget(resetUsages);
        buttonMappedUsages->addItem("Disabled");
        for (int i = 0; i < 16; i++)
        {
            buttonUsages->addItem(QString("Button %1").arg(i+1));
            buttonMappedUsages->addItem(QString("Button %1").arg(i+1));
        }
        connect(buttonUsages, SIGNAL(currentIndexChanged(int)), this, SLOT(onButtonUsageChanged(int)));
        connect(buttonMappedUsages, SIGNAL(currentIndexChanged(int)), this, SLOT(onButtonMappedUsageChanged(int)));
        connect(getRemapping, SIGNAL(pressed()), this, SLOT(onGetRemapping()));
        connect(resetUsages, SIGNAL(pressed()), this, SLOT(onResetButtonUsages()));
    
        buttonMapLayout->addSpacerItem(new QSpacerItem(0,0));
        buttonMapLayout->setStretch(buttonMapLayout->count(), 2);

        vButtonLayout->addWidget(group);
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

void f8110::onButtonReport(unsigned short newButtonState)
{
    notifyText(QString("0x%1").arg(newButtonState, 0, 16));
}

void f8110::onToggleSpy(void)
{
    if (isSpying)
    {
        if (!feature8110->StopSpy())
        {
            notifyText("ERROR: feature8110->StopSpy");
        }
        uiSpyToggle->setText("Start Spying");
    }
    else
    {
        if (!feature8110->StartSpy())
        {
            notifyText("ERROR: feature8110->StartSpy");
        }
        uiSpyToggle->setText("Stop Spying");
    }
    isSpying = !isSpying;
    
    notifyText(isSpying ? "Now spying" : "Not spying");
}

void f8110::onButtonUsageChanged(int index)
{
    // Show the current selection
    int hidButtonMappedUsage = hidButtonMap[index];
    if (hidButtonMappedUsage <= 0)
    {
        notifyText(QString("Invalid hidButtonMappedUsage (%1) at index %2").arg(hidButtonMappedUsage).arg(index));
        return;
    }
    
    buttonMappedUsages->blockSignals(true);
    buttonMappedUsages->setCurrentIndex(hidButtonMappedUsage);
    buttonMappedUsages->blockSignals(false);
}

void f8110::onButtonMappedUsageChanged(int index)
{
    int hidButtonUsage = buttonUsages->currentIndex() + 1;
    int hidButtonMappedUsage = index;
    
    if (hidButtonUsage > 16)
    {
        notifyText(QString("Invalid hidButtonUsage (%1) at index %2").arg(hidButtonUsage).arg(index));
        return;
    }
    
    if (hidButtonMap[hidButtonUsage-1] != hidButtonMappedUsage)
    {
        hidButtonMap[hidButtonUsage-1] = hidButtonMappedUsage;
        notifyText(QString("Button %1 is now mapped to usage %2").arg(hidButtonUsage).arg(hidButtonMappedUsage));
        feature8110->SetButtonMap(hidButtonMap);
    }
    
    dumpButtonUsages();
}

void f8110::onGetRemapping(void)
{
    notifyText("Getting button mapping from device");
    hidButtonMap.resize(16);
    for(int i = 0; i < 16; i++)
    {
        hidButtonMap[i] = 0;
    }
    feature8110->GetButtonMap(hidButtonMap);
    buttonUsages->setCurrentIndex(0);
    buttonMappedUsages->blockSignals(true);
    buttonMappedUsages->setCurrentIndex(hidButtonMap[0]);
    buttonMappedUsages->blockSignals(false);
    dumpButtonUsages();
}


void f8110::onResetButtonUsages(void)
{
    for (int i = 0; i < 16; i++)
    {
        hidButtonMap[i] = i+1;
    }
    buttonUsages->setCurrentIndex(0);
    buttonMappedUsages->blockSignals(true);
    buttonMappedUsages->setCurrentIndex(hidButtonMap[0]);
    buttonMappedUsages->blockSignals(false);
    feature8110->SetButtonMap(hidButtonMap);
    dumpButtonUsages();
}

void f8110::notifyText(const QString &text, bool insertCRLF)
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

void f8110::refreshData(void)
{
    buttonCount = 0;
    
    hidButtonMap.resize(16);
    for(int i = 0; i < 16; i++)
    {
        hidButtonMap[i] = 0;
    }

    feature8110->GetButtonCount(&buttonCount);
    feature8110->GetButtonMap(hidButtonMap);
    
    dumpButtonUsages();
}

void f8110::refreshUI(void)
{
    QString buttonCountText;
    if (buttonCount)
    {
        uiButtonCount->setText(QString("%1").arg(buttonCount));
    }
    else
    {
        uiButtonCount->setText("<ERROR>");
    }
    
    buttonUsages->setCurrentIndex(0);
    buttonMappedUsages->blockSignals(true);
    buttonMappedUsages->setCurrentIndex(hidButtonMap[0]);
    buttonMappedUsages->blockSignals(false);
}

void f8110::dumpButtonUsages(void)
{
    notifyText("--------------------------------------");
    for(int i = 0; i < 16; i++)
    {
        notifyText(QString("Button %1 is mapped to usage %2").arg(i+1).arg(hidButtonMap[i]));
    }
    notifyText("--------------------------------------");
}

void f8110::onNotificationContextMenu(const QPoint &pos)
{
    QMenu popup(this);
    popup.addAction("Clear", this, SLOT(onClearNotifications()));
    popup.exec(notifications->mapToGlobal(pos));
}

void f8110::onClearNotifications(void)
{
    notifications->clear();
}


