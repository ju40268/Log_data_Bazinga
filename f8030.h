#pragma once

#include <QWidget>
#include <QTextEdit>
#include "feature.h"
#include "devio.h"
#include <vector>

using devio::IFeature;
using devio::IFeature8030MR;
using devio::Byte;
using devio::Subscriber;
using std::vector;


class f8030 : public QWidget,
    public Subscriber<devio::IFeature8030MR::Report>
{
    Q_OBJECT

public:
    explicit f8030(shared_ptr<devio::IFeature8030MR> f, Feature *base);
    virtual void onButtonReport(const bool &pressed);

private slots:
    void onMRChecked(int state);
    void onContextMenu(const QPoint& pt);
    void onClearNotifications(void);

private:
    void createUI(void);
    void notifyText(const QString &text, bool insertCRLF = false);

    shared_ptr<devio::IFeature8030MR> feature8030;
    Feature *baseFeature;

    QTextEdit *notifications;
};
