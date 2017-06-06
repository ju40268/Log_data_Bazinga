#ifndef F8123_CONDITIONFORCEWIDGET_H
#define F8123_CONDITIONFORCEWIDGET_H

#include "f8123_forcewidget.h"

namespace Ui {
class f8123_ConditionForceWidget;
}

class f8123_ConditionForceWidget : public f8123_ForceWidget
{
    Q_OBJECT

public:
    explicit f8123_ConditionForceWidget(shared_ptr<devio::IFeature8123ForceFeedback> f,
                                        devio::IFeature8123ForceFeedback::EffectType effectType,
                                        QWidget *parent = 0);
    ~f8123_ConditionForceWidget(void);

    virtual void refreshButtons(void);
    virtual void setForceName(const QString &forceName);

protected slots:
    void onDownload(void);

private:
    Ui::f8123_ConditionForceWidget *ui;
};

#endif // F8123_CONDITIONFORCEWIDGET_H
