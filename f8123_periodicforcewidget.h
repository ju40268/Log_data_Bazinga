#ifndef F8123_PERIODICFORCEWIDGET_H
#define F8123_PERIODICFORCEWIDGET_H

#include "f8123_forcewidget.h"

namespace Ui {
class f8123_PeriodicForceWidget;
}

class f8123_PeriodicForceWidget : public f8123_ForceWidget
{
    Q_OBJECT

public:
    explicit f8123_PeriodicForceWidget(shared_ptr<devio::IFeature8123ForceFeedback> f,
                                       devio::IFeature8123ForceFeedback::EffectType effectType,
                                       QWidget *parent = 0);
    ~f8123_PeriodicForceWidget(void);

    virtual void refreshButtons(void);
    virtual void setForceName(const QString &forceName);

protected slots:
    void onDownload(void);

private:
    Ui::f8123_PeriodicForceWidget *ui;
};

#endif // F8123_PERIODICFORCEWIDGET_H
