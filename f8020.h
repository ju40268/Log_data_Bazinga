#pragma once

#include <QWidget>
#include <QComboBox>
#include <QTextEdit>
#include "feature.h"
#include "devio.h"
#include <vector>

using devio::IFeature;
using devio::IFeature8020Mkeys;
using devio::Byte;
using devio::Subscriber;
using std::vector;


class f8020 :   public QWidget,
    public Subscriber<devio::IFeature8020Mkeys::Report>
{
    Q_OBJECT

public:
    explicit f8020(shared_ptr<devio::IFeature8020Mkeys> f, Feature *base);
    virtual void onButtonReport(const Byte &buttonState);

protected slots:
    void onMStateChanged(int itemIdx);
    void onContextMenu(const QPoint& pt);
    void onClearNotifications(void);

private:
    void createUI(void);
    void refreshUI(void);
    void notifyText(const QString &text, bool insertCRLF = false);

    shared_ptr<devio::IFeature8020Mkeys> feature8020;
    Feature *baseFeature;
    Byte prevState;

    QComboBox *mStates_Combo;
    QTextEdit *notifications;
};
