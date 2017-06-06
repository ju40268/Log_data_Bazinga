#include "feature.h"
#include "f8300.h"
#include <QMenu>
#include <QGroupBox>
#include <QValidator>
#include "xlog/xlog.h"

f8300::f8300(shared_ptr<devio::IFeature8300Sidetone> f, Feature* base) :
QWidget(),
feature8300(f),
baseFeature(base),
notifications(NULL)
{
    createUI();
}


void f8300::createUI(void)
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    setLayout(vLayout);

    {
        QGroupBox* group = new QGroupBox("Sidetone");
        QVBoxLayout* dataLayout = new QVBoxLayout;
        group->setLayout(dataLayout);

        QHBoxLayout* setSTLayout = new QHBoxLayout;
        setSidetoneLevel = new QPushButton("Set Sidetone Level");
        connect(setSidetoneLevel, SIGNAL(clicked()), this, SLOT(onSetSidetoneLevel()));
        levelEdit = new QLineEdit;
        QValidator *validator = new QIntValidator(0, 100);
        levelEdit->setValidator(validator);

        setSTLayout->addWidget(setSidetoneLevel);
        setSTLayout->addWidget(levelEdit);

        getSidetoneLevel = new QPushButton("Get Sidetone Level");
        connect(getSidetoneLevel, SIGNAL(clicked()), this, SLOT(onGetSidetoneLevel()));

        dataLayout->addLayout(setSTLayout);
        dataLayout->addWidget(getSidetoneLevel);

        vLayout->addWidget(group);
    }

    // Notifications
    {
        notifications = new QTextEdit;
        notifications->setReadOnly(true);
        notifications->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(notifications, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onNotificationContextMenu(const QPoint&)));
        vLayout->addWidget(notifications);
    }

    vLayout->setStretch(vLayout->count() - 1, 1);

    // Get current settings
    onGetSidetoneLevel();
}

void f8300::onGetSidetoneLevel(void)
{
    int levelValue = 0;
    if (feature8300->getSidetoneLevel(levelValue))
    {
        notifyText(QString("Sidetone is %1").arg(levelValue));
    }
    else
    {
        notifyText("getSidetoneLevel failed");
    }
}

void f8300::onSetSidetoneLevel(void)
{
    int level = (int)(levelEdit->text().toUShort(NULL, 0));
    if (!feature8300->setSidetoneLevel(level))
    {
        notifyText("setSidetoneLevel failed");
        return;
    }

    notifyText(QString("Sidetone changed to %1").arg(level));
}


void f8300::notifyText(const QString &text, bool insertCRLF)
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

void f8300::onContextMenu(const QPoint &pos)
{
    QMenu popup(this);
    popup.addAction("Clear", this, SLOT(onClearNotifications()));
    popup.exec(notifications->mapToGlobal(pos));
}

void f8300::onClearNotifications(void)
{
    notifications->clear();
}