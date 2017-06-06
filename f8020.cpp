#include "feature.h"
#include "f8020.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include <QGroupBox>
#include <QMenu>

using std::string;
using std::vector;
using devio::Byte;

using devio::Delivery;

#define BITS_PER_BYTE       8

//*************************************************************************
//
// f8020::f8020
//
//*************************************************************************

f8020::f8020(shared_ptr<devio::IFeature8020Mkeys> f, Feature *base)
    : QWidget(),
    feature8020(f),
    baseFeature(base)
{
    createUI();
    refreshUI();

    prevState = 0;
    subscribe(feature8020, Delivery::Immediate);
}


//*************************************************************************
//
// f8020::createUI
//
//*************************************************************************

void f8020::createUI(void)
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    setLayout(vLayout);

    QGroupBox *group = new QGroupBox("Set M state:");
    QVBoxLayout *dataLayout = new QVBoxLayout;
    group->setLayout(dataLayout);

    mStates_Combo = new QComboBox;
    connect(mStates_Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(onMStateChanged(int)));

    dataLayout->addWidget(mStates_Combo);

    vLayout->addWidget(group);

    notifications = new QTextEdit;
    notifications->setReadOnly(true);
    notifications->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(notifications, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenu(const QPoint&)));

    vLayout->addWidget(notifications);
}


//*************************************************************************
//
// f8020::refreshUI
//
//*************************************************************************

void f8020::refreshUI(void)
{
    int count=0;
    bool ok = feature8020->GetCount(count);
    if(ok)
    {
        mStates_Combo->blockSignals(true);
        mStates_Combo->clear();

        for(int i=1; i<=count; i++)
        {
            mStates_Combo->addItem(QString("M%1").arg(i), i);
        }

        mStates_Combo->blockSignals(false);
    }

    onMStateChanged(0);
}


//*************************************************************************
//
// f8020::notifyText
//
//*************************************************************************

void f8020::notifyText(const QString &text, bool insertCRLF /*= false*/)
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


//*************************************************************************
//
// f8020::onMStateChanged
//
//*************************************************************************

void f8020::onMStateChanged(int itemIdx)
{
    int mstate = mStates_Combo->itemData(itemIdx).toInt();
    if(!feature8020->SetMkeyLED((IFeature8020Mkeys::Mkey)mstate))
    {
        notifyText(QString("ERROR: setMkeyLED failed (M%1)").arg(mstate));
    }
    else
    {
        notifyText(QString("M state is now set to M%1").arg(mstate));
    }
}


//*************************************************************************
//
// f8020::onContextMenu
//
//*************************************************************************

void f8020::onContextMenu(const QPoint& pt)
{
    QMenu popup(this);
    popup.addAction("Clear", this, SLOT(onClearNotifications()));
    popup.exec(notifications->mapToGlobal(pt));
}


//*************************************************************************
//
// f8020::onClearNotifications
//
//*************************************************************************

void f8020::onClearNotifications(void)
{
    notifications->clear();
}


//*************************************************************************
//
// f8020::onButtonReport
//
//*************************************************************************

void f8020::onButtonReport(const Byte &buttonState)
{
    Byte r = buttonState ^ prevState;
    if(r != 0)
    {
        for(int i=0; i<BITS_PER_BYTE; i++)
        {
            if((bool)(r.bit(i)) && (bool)(buttonState.bit(i)))
            {
                notifyText(QString("M%1 is pressed").arg(i+1));
                feature8020->SetMkeyLED((IFeature8020Mkeys::Mkey)(i+1));
            }
        }
        prevState = buttonState;
    }
}
