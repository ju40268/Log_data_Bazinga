#ifndef F8123_CONSTANTFORCEWIDGET_H
#define F8123_CONSTANTFORCEWIDGET_H

#include "f8123_forcewidget.h"
#include "devio.h"

namespace Ui {
class f8123_ConstantForceWidget;
}

class f8123_ConstantForceWidget : public f8123_ForceWidget
{
    Q_OBJECT

public:
    explicit f8123_ConstantForceWidget(shared_ptr<devio::IFeature8123ForceFeedback> f,
                                       devio::IFeature8123ForceFeedback::EffectType effectType,
                                       QWidget *parent = 0);
    ~f8123_ConstantForceWidget();

    virtual void refreshButtons(void);
    virtual void setForceName(const QString &forceName);

protected slots:
    void onDownload(void);

private:
    Ui::f8123_ConstantForceWidget *ui;
};

#endif // F8123_CONSTANTFORCEWIDGET_H
