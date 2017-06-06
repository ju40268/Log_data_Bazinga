#pragma once

#include <QtWidgets>
#include "feature.h"
#include "devio.h"
#include <vector>

using devio::IFeature;
using devio::IFeature8010Gkey;
using devio::Byte;
using devio::Subscriber;
using std::vector;

#define BUTTON_REPORT_SIZE  4
#define BITS_PER_BYTE       8

class f8010 :   public QWidget,
                public Subscriber<devio::IFeature8010Gkey::Report>
{
    Q_OBJECT

public:
    explicit f8010(shared_ptr<devio::IFeature8010Gkey> f, Feature* base);
    virtual void onButtonReport(const vector<Byte>& buttonState);

protected:
    void notifyText(const QString &text, bool insertCRLF = false);

protected slots:
    void onSWControlChanged(int state);
    void onContextMenu(const QPoint& pt);
    void onClearNotifications(void);

private:
    shared_ptr<devio::IFeature8010Gkey> feature8010;
    Feature *baseFeature;
    int m_count;
    vector<Byte> prevStates;

    QTextEdit *notifications;
};
