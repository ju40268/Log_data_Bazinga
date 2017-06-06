#ifndef F8123_RAMPFORCEWIDGET_H
#define F8123_RAMPFORCEWIDGET_H

#include "f8123_forcewidget.h"

namespace Ui {
class f8123_RampForceWidget;
}

class f8123_RampForceWidget : public f8123_ForceWidget
{
    Q_OBJECT

public:
    explicit f8123_RampForceWidget(shared_ptr<devio::IFeature8123ForceFeedback> f,
                                        devio::IFeature8123ForceFeedback::EffectType effectType,
                                        QWidget *parent = 0);
    ~f8123_RampForceWidget(void);

    virtual void refreshButtons(void);
    virtual void setForceName(const QString &forceName);

protected slots:
    void onDownload(void);

private:
    Ui::f8123_RampForceWidget *ui;
};

#endif // F8123_RAMPFORCEWIDGET_H
