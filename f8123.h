#pragma once

#include "ui_f8123.h"
#include "feature.h"
#include <vector>
#include "devio.h"
#include "f8123_forcewidget.h"

using devio::IFeature;
using devio::IFeature8123ForceFeedback;
using devio::Byte;
using devio::Subscriber;
using std::vector;

class f8123 : public QWidget,
              public Subscriber<devio::IFeature8123ForceFeedback::ForceNotification>
{
    Q_OBJECT
public:
    explicit f8123(shared_ptr<devio::IFeature8123ForceFeedback> f, Feature* base);

protected:
    void notifyText(const QString &text, bool insertCRLF = false);
    void addForceWidget(f8123_ForceWidget *forceWidget, const QString &forceAndTabName);

    virtual void onExpiredForces(const vector<unsigned> &expiredForces);

protected slots:
    void onGetInfo(void);
    void onResetAll(void);
    void onCreateForce(void);
    void onNotificationContextMenu(const QPoint &pos);
    void onClearNotifications(void);
    void onEffectDestroyed(void);
    void onGetAperture(void);
    void onSetAperture(void);
    void onApertureChanged(int newAperture);
    void onGetGlobalGains(void);
    void onSetGlobalGains(void);
    void onGlobalGainChanged(int newGlobalGain);
    void onGlobalGainChanged(double newGlobalGain);
    void onGlobalBoostChanged(int newGlobalBoost);
    void onGlobalBoostChanged(double newGlobalBoost);

private:
    shared_ptr<devio::IFeature8123ForceFeedback> feature8123;
    Feature* baseFeature;
    Ui_f8123 ui;
};
