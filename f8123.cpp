#include "feature.h"
#include "f8123.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include <QMenu>
#include <QGroupBox>
#include "f8123_constantforcewidget.h"
#include "f8123_periodicforcewidget.h"
#include "f8123_conditionforcewidget.h"
#include "f8123_rampforcewidget.h"

using devio::Delivery;
using devio::IFeature8123ForceFeedback;

enum CreateEffectType {
    Constant,
    PeriodicSine,
    PeriodicSquare,
    PeriodicTriangle,
    PeriodicSawtoothUp,
    PeriodicSawtoothDown,
    ConditionSpring,
    ConditionDamper,
    ConditionFriction,
    ConditionInertia,
    Ramp
};

f8123::f8123(shared_ptr<devio::IFeature8123ForceFeedback> f, Feature* base) :
    QWidget(),
    feature8123(f),
    baseFeature(base)
{
    ui.setupUi(this);

    connect(ui.getInfoButton, SIGNAL(clicked()), this, SLOT(onGetInfo()));
    connect(ui.resetAllButton, SIGNAL(clicked()), this, SLOT(onResetAll()));
    connect(ui.notifications, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onNotificationContextMenu(QPoint)));

    ui.createEffectType->addItem("Constant Force", Constant);
    ui.createEffectType->addItem("Periodic Sine", PeriodicSine);
    ui.createEffectType->addItem("Periodic Square", PeriodicSquare);
    ui.createEffectType->addItem("Periodic Triangle", PeriodicTriangle);
    ui.createEffectType->addItem("Periodic SawtoothUp", PeriodicSawtoothUp);
    ui.createEffectType->addItem("Periodic SawtoothDown", PeriodicSawtoothDown);
    ui.createEffectType->addItem("Spring", ConditionSpring);
    ui.createEffectType->addItem("Damper", ConditionDamper);
    ui.createEffectType->addItem("Friction", ConditionFriction);
    ui.createEffectType->addItem("Inertia", ConditionInertia);
    ui.createEffectType->addItem("Ramp", Ramp);

    connect(ui.createEffectButton, SIGNAL(clicked()), this, SLOT(onCreateForce()));

    connect(ui.getApertureButton, SIGNAL(clicked()), this, SLOT(onGetAperture()));
    connect(ui.setApertureButton, SIGNAL(clicked()), this, SLOT(onSetAperture()));
    connect(ui.apertureSlider, SIGNAL(valueChanged(int)), this, SLOT(onApertureChanged(int)));
    connect(ui.apertureSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onApertureChanged(int)));

    connect(ui.getGlobalGainsButton, SIGNAL(clicked()), this, SLOT(onGetGlobalGains()));
    connect(ui.setGlobalGainsButton, SIGNAL(clicked()), this, SLOT(onSetGlobalGains()));
    connect(ui.globalGainSlider, SIGNAL(valueChanged(int)), this, SLOT(onGlobalGainChanged(int)));
    connect(ui.globalGainDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onGlobalGainChanged(double)));
    connect(ui.globalBoostSlider, SIGNAL(valueChanged(int)), this, SLOT(onGlobalBoostChanged(int)));
    connect(ui.globalBoostDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onGlobalBoostChanged(double)));

    subscribe(feature8123);
}

void f8123::notifyText(const QString &text, bool insertCRLF)
{
    // Jump to the end (to restore the cursor)
    QTextCursor tc = ui.notifications->textCursor();
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    ui.notifications->setTextCursor(tc);

    ui.notifications->insertPlainText(text + QString("\n"));
    if (insertCRLF)
    {
        ui.notifications->append("\n");
    }

    // Jump to the end (to scroll to the end)
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    ui.notifications->setTextCursor(tc);
}

void f8123::addForceWidget(f8123_ForceWidget *forceWidget, const QString &forceAndTabName)
{
    connect(forceWidget, SIGNAL(effectDestroyed()), this, SLOT(onEffectDestroyed()));
    forceWidget->setForceName(forceAndTabName);
    int index = ui.effectsTabWidget->addTab(forceWidget, forceAndTabName);
    ui.effectsTabWidget->setCurrentIndex(index);
}

void f8123::onExpiredForces(const vector<unsigned> &expiredForces)
{
    QStringList expiredList;
    Q_FOREACH(unsigned forceId, expiredForces)
    {
        expiredList.append(QString::number(forceId));
    }
    notifyText(QString("Expired: %1").arg(expiredList.join(",")));
}

void f8123::onGetInfo(void)
{
    unsigned maxEffectCount, forceAxes;

    if (feature8123->getInfo(&maxEffectCount, &forceAxes))
    {
        ui.maxEffectCountLabel->setText(QString::number(maxEffectCount));
        ui.forceAxesLabel->setText(QString::number(forceAxes, 16));
    }
    else
    {
        notifyText("Failed to perform getInfo() call!");
    }
}

void f8123::onResetAll(void)
{
    feature8123->resetAll();
    while (ui.effectsTabWidget->count())
    {
        ui.effectsTabWidget->widget(0)->deleteLater();
        ui.effectsTabWidget->removeTab(0);
    }
    notifyText("ResetAll sent.");
}

void f8123::onCreateForce(void)
{
    int requestedType = ui.createEffectType->currentData().toInt();
    switch (requestedType)
    {
    case Constant:
        addForceWidget(new f8123_ConstantForceWidget(feature8123, IFeature8123ForceFeedback::Constant, this), "Constant");
        break;

    case PeriodicSine:
        addForceWidget(new f8123_PeriodicForceWidget(feature8123, IFeature8123ForceFeedback::Sine, this), "Sine");
        break;

    case PeriodicSquare:
        addForceWidget(new f8123_PeriodicForceWidget(feature8123, IFeature8123ForceFeedback::Square, this), "Square");
        break;

    case PeriodicTriangle:
        addForceWidget(new f8123_PeriodicForceWidget(feature8123, IFeature8123ForceFeedback::Triangle, this), "Triangle");
        break;

    case PeriodicSawtoothUp:
        addForceWidget(new f8123_PeriodicForceWidget(feature8123, IFeature8123ForceFeedback::SawtoothUp, this), "SawtoothUp");
        break;

    case PeriodicSawtoothDown:
        addForceWidget(new f8123_PeriodicForceWidget(feature8123, IFeature8123ForceFeedback::SawtoothDown, this), "SawtoothDown");
        break;

    case ConditionSpring:
        addForceWidget(new f8123_ConditionForceWidget(feature8123, IFeature8123ForceFeedback::Spring, this), "Spring");
        break;

    case ConditionDamper:
        addForceWidget(new f8123_ConditionForceWidget(feature8123, IFeature8123ForceFeedback::Damper, this), "Damper");
        break;

    case ConditionFriction:
        addForceWidget(new f8123_ConditionForceWidget(feature8123, IFeature8123ForceFeedback::Friction, this), "Friction");
        break;

    case ConditionInertia:
        addForceWidget(new f8123_ConditionForceWidget(feature8123, IFeature8123ForceFeedback::Inertia, this), "Inertia");
        break;

    case Ramp:
        addForceWidget(new f8123_RampForceWidget(feature8123, IFeature8123ForceFeedback::Ramp, this), "Ramp");
        break;

    default:
        break;
    }
}

void f8123::onEffectDestroyed(void)
{
    QWidget *forceWidget = qobject_cast<QWidget *>(sender());
    if (forceWidget)
    {
        int index = ui.effectsTabWidget->indexOf(forceWidget);
        ui.effectsTabWidget->removeTab(index);
        forceWidget->deleteLater();
    }
}

void f8123::onGetAperture(void)
{
    unsigned aperture = 0;
    if (feature8123->getAperture(&aperture))
    {
        ui.apertureSpinBox->setValue(aperture);
    }
    else
    {
        notifyText("Failed to perform getAperture() call!");
    }
}

void f8123::onSetAperture(void)
{
    unsigned aperture = ui.apertureSpinBox->value();
    if (!feature8123->setAperture(aperture))
    {
        notifyText("Failed to perform setAperture() call!");
    }
}

void f8123::onApertureChanged(int newAperture)
{
    bool wasEnabled = ui.apertureSpinBox->blockSignals(true);
    ui.apertureSpinBox->setValue(newAperture);
    ui.apertureSpinBox->blockSignals(wasEnabled);

    wasEnabled = ui.apertureSlider->blockSignals(true);
    ui.apertureSlider->setValue(newAperture);
    ui.apertureSlider->blockSignals(wasEnabled);

    onSetAperture();
}

void f8123::onGetGlobalGains(void)
{
    float globalGain = 0, globalBoost = 0;
    if (feature8123->getGlobalGains(&globalGain, &globalBoost))
    {
        ui.globalGainDoubleSpinBox->setValue(globalGain);
        ui.globalBoostDoubleSpinBox->setValue(globalBoost);
    }
    else
    {
        notifyText("Failed to perform getGlobalGains() call!");
    }
}

void f8123::onSetGlobalGains(void)
{
    float globalGain = ui.globalGainDoubleSpinBox->value();
    float globalBoost = ui.globalBoostDoubleSpinBox->value();
    if (!feature8123->setGlobalGains(globalGain, globalBoost))
    {
        notifyText("Failed to perform setGlobalGains() call!");
    }
}

void f8123::onGlobalGainChanged(int newGlobalGain)
{
    bool wasEnabled = ui.globalGainDoubleSpinBox->blockSignals(true);
    ui.globalGainDoubleSpinBox->setValue(newGlobalGain / 100.0);
    ui.globalGainDoubleSpinBox->blockSignals(wasEnabled);

    onSetGlobalGains();
}

void f8123::onGlobalGainChanged(double newGlobalGain)
{
    bool wasEnabled = ui.globalGainSlider->blockSignals(true);
    ui.globalGainSlider->setValue(newGlobalGain * 100);
    ui.globalGainSlider->blockSignals(wasEnabled);

    onSetGlobalGains();
}

void f8123::onGlobalBoostChanged(int newGlobalBoost)
{
    bool wasEnabled = ui.globalBoostDoubleSpinBox->blockSignals(true);
    ui.globalBoostDoubleSpinBox->setValue(newGlobalBoost / 100.0);
    ui.globalBoostDoubleSpinBox->blockSignals(wasEnabled);

    onSetGlobalGains();
}

void f8123::onGlobalBoostChanged(double newGlobalBoost)
{
    bool wasEnabled = ui.globalBoostSlider->blockSignals(true);
    ui.globalBoostSlider->setValue(newGlobalBoost * 100);
    ui.globalBoostSlider->blockSignals(wasEnabled);

    onSetGlobalGains();
}

void f8123::onNotificationContextMenu(const QPoint &pos)
{
    QMenu popup(this);
    popup.addAction("Clear", this, SLOT(onClearNotifications()));
    popup.exec(ui.notifications->mapToGlobal(pos));
}

void f8123::onClearNotifications(void)
{
    ui.notifications->clear();
}
