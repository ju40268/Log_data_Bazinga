#include "feature.h"
#include "f8010.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"

using std::string;
using std::vector;
using devio::Byte;

using devio::Delivery;


//*************************************************************************
//
// f8010::f8010
//
//*************************************************************************

f8010::f8010(shared_ptr<devio::IFeature8010Gkey> f, Feature* base)
    : QWidget(),
    feature8010(f),
    baseFeature(base)
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    setLayout(vLayout);

    QLabel *description = new QLabel(baseFeature->description);

    m_count=0;
    bool ok = feature8010->GetCount(m_count);
    QLabel *countLabel = new QLabel(stringbuilder() << "GetCount(count=" << m_count << ") -> " << describe_err(ok));

    IFeature8010Gkey::GkeyLayout layout;
    ok = feature8010->GetPhysicalLayout(layout);
    QLabel *layoutLabel = new QLabel(stringbuilder() << "GetPhysicalLayout(layout=" << layout << ") -> " << describe_err(ok));

    QCheckBox *swControl = new QCheckBox("Enable Software Control");
    swControl->setChecked(false);
    feature8010->EnableSoftwareControl(false);
    connect(swControl, SIGNAL(stateChanged(int)), this, SLOT(onSWControlChanged(int)));

    notifications = new QTextEdit;
    notifications->setReadOnly(true);
    notifications->setWordWrapMode(QTextOption::NoWrap);
    notifications->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(notifications, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenu(const QPoint&)));

    vLayout->addWidget(description);
    vLayout->addWidget(countLabel);
    vLayout->addWidget(layoutLabel);
    vLayout->addWidget(swControl);
    vLayout->addWidget(notifications);

    prevStates = vector<Byte>(4, 0);

    subscribe(feature8010, Delivery::Immediate);
}


//*************************************************************************
//
// f8010::onButtonReport
//
//*************************************************************************

void f8010::onButtonReport(const vector<Byte>& buttonState)
{
    if(buttonState.size() < BUTTON_REPORT_SIZE)
    {
        return;
    }

    for(int idx=0; idx<m_count; idx++)
    {
        int vectorIdx = idx/8;
        int bitIdx = idx%8;

        if((bool)(buttonState[vectorIdx].bit(bitIdx)) != (bool)(prevStates[vectorIdx].bit(bitIdx)))
        {
            QString text;
            text.append(QString("G%1 is %2")
                .arg(idx + 1)
                .arg(buttonState[vectorIdx].bit(bitIdx) ? "down" : "up"));

            notifyText(text);
        }
    }

    prevStates = buttonState;
}


//*************************************************************************
//
// f8010::notifyText
//
//*************************************************************************

void f8010::notifyText(const QString &text, bool insertCRLF /*= false*/)
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
// f8010::onContextMenu
//
//*************************************************************************

void f8010::onContextMenu(const QPoint& pt)
{
    QMenu popup(this);
    popup.addAction("Clear", this, SLOT(onClearNotifications()));
    popup.exec(notifications->mapToGlobal(pt));
}


//*************************************************************************
//
// f8010::onClearNotifications
//
//*************************************************************************

void f8010::onClearNotifications(void)
{
    notifications->clear();
}


//*************************************************************************
//
// f8010::onSWControlChanged
//
//*************************************************************************

void f8010::onSWControlChanged(int state)
{
    feature8010->EnableSoftwareControl(state != 0);
}
