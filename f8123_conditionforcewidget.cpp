#include "f8123_conditionforcewidget.h"
#include "ui_f8123_conditionforcewidget.h"

f8123_ConditionForceWidget::f8123_ConditionForceWidget(shared_ptr<devio::IFeature8123ForceFeedback> f,
                                                       devio::IFeature8123ForceFeedback::EffectType effectType,
                                                       QWidget *parent) :
    f8123_ForceWidget(f, effectType, parent),
    ui(new Ui::f8123_ConditionForceWidget)
{
    ui->setupUi(this);

    setPlayStopPauseButtons(ui->playButton, ui->stopButton, ui->pauseButton);

    connect(ui->destroyEffectButton, SIGNAL(clicked()), this, SLOT(onDestroyEffect()));
    connect(ui->downloadAndStartButton, SIGNAL(clicked()), this, SLOT(onDownload()));
    connect(ui->downloadButton, SIGNAL(clicked()), this, SLOT(onDownload()));
}

f8123_ConditionForceWidget::~f8123_ConditionForceWidget(void)
{
    delete ui;
}

void f8123_ConditionForceWidget::refreshButtons(void)
{
    f8123_ForceWidget::refreshButtons();

    ui->downloadAndStartButton->setEnabled(0 == m_effectId);
}

void f8123_ConditionForceWidget::setForceName(const QString &forceName)
{
    ui->effectTypeLabel->setText(forceName);
}

void f8123_ConditionForceWidget::onDownload(void)
{
    bool startNow = (sender() == ui->downloadAndStartButton);
    unsigned duration = ui->durationSpinBox->value();
    unsigned startDelay = ui->startDelaySpinBox->value();
    float negSaturation = ui->negSaturationSpinBox->value();
    float negSlope = ui->negSlopeSpinBox->value();
    float deadzone = ui->deadzoneSpinBox->value();
    float offset = ui->offsetSpinBox->value();
    float posSlope = ui->posSlopeSpinBox->value();
    float posSaturation = ui->posSaturationSpinBox->value();
    feature8123->downloadEffectCondition(&m_effectId, m_effectType, devio::IFeature8123ForceFeedback::PrimaryAxis, startNow,
                                         duration, startDelay,
                                         negSaturation, negSlope, deadzone, offset, posSlope, posSaturation);
    ui->idLabel->setText(QString::number(m_effectId));
    refreshButtons();
}
