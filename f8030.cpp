#include "feature.h"
#include "f8030.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include <QCheckBox>
#include <QMenu>

using std::string;
using devio::Delivery;

#define BITS_PER_BYTE       8



//*************************************************************************
//
// f8030::f8030
//
//*************************************************************************

f8030::f8030(shared_ptr<devio::IFeature8030MR> f, Feature *base)
    : QWidget(),
    feature8030(f),
    baseFeature(base)
{
    createUI();
    subscribe(feature8030, Delivery::Immediate);
}


//*************************************************************************
//
// f8030::onButtonReport
//
//*************************************************************************

void f8030::onButtonReport(const bool &pressed)
{
    notifyText(QString("MR is %1").arg(pressed ? "pressed" : "released"));
    feature8030->SetLED(pressed ? true : false);
}


//*************************************************************************
//
// f8030::createUI
//
//*************************************************************************

void f8030::createUI(void)
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    setLayout(vLayout);

    QCheckBox *mrCheck = new QCheckBox("Set MR");
    mrCheck->setChecked(false);
    connect(mrCheck, SIGNAL(stateChanged(int)), this, SLOT(onMRChecked(int)));

    notifications = new QTextEdit;
    notifications->setReadOnly(true);
    notifications->setWordWrapMode(QTextOption::NoWrap);
    connect(notifications, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenu(const QPoint&)));

    vLayout->addWidget(mrCheck);
    vLayout->addWidget(notifications);
}


//*************************************************************************
//
// f8030::notifyText
//
//*************************************************************************

void f8030::notifyText(const QString &text, bool insertCRLF /*= false*/)
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
// f8030::onMRChecked
//
//*************************************************************************

void f8030::onMRChecked(int state)
{
    bool on = (state != 0);

    feature8030->SetLED(on);
    notifyText(QString("MR is turned %1").arg(on ? "on" : "off"));
}


//*************************************************************************
//
// f8030::onContextMenu
//
//*************************************************************************

void f8030::onContextMenu(const QPoint& pt)
{
    QMenu popup(this);
    popup.addAction("Clear", this, SLOT(onClearNotifications()));
    popup.exec(notifications->mapToGlobal(pt));
}


//*************************************************************************
//
// f8030::onClearNotifications
//
//*************************************************************************

void f8030::onClearNotifications(void)
{
    notifications->clear();
}
