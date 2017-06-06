#include "f8123_periodicforcewidget.h"
#include "ui_f8123_periodicforcewidget.h"

f8123_PeriodicForceWidget::f8123_PeriodicForceWidget(shared_ptr<devio::IFeature8123ForceFeedback> f,
                                                     devio::IFeature8123ForceFeedback::EffectType effectType,
                                                     QWidget *parent) :
    f8123_ForceWidget(f, effectType, parent),
    ui(new Ui::f8123_PeriodicForceWidget)
{
    ui->setupUi(this);

    setPlayStopPauseButtons(ui->playButton, ui->stopButton, ui->pauseButton);

    connect(ui->destroyEffectButton, SIGNAL(clicked()), this, SLOT(onDestroyEffect()));
    connect(ui->downloadAndStartButton, SIGNAL(clicked()), this, SLOT(onDownload()));
    connect(ui->downloadButton, SIGNAL(clicked()), this, SLOT(onDownload()));
}

f8123_PeriodicForceWidget::~f8123_PeriodicForceWidget(void)
{
    delete ui;
}

void f8123_PeriodicForceWidget::refreshButtons(void)
{
    f8123_ForceWidget::refreshButtons();

    ui->downloadAndStartButton->setEnabled(0 == m_effectId);
}

void f8123_PeriodicForceWidget::setForceName(const QString &forceName)
{
    ui->effectTypeLabel->setText(forceName);
}

void f8123_PeriodicForceWidget::onDownload(void)
{
    bool startNow = (sender() == ui->downloadAndStartButton);
    unsigned duration = ui->durationSpinBox->value();
    unsigned startDelay = ui->startDelaySpinBox->value();
    float level = ui->levelSpinBox->value();
    float offset = ui->offsetSpinBox->value();
    unsigned waveLength = ui->waveLengthSpinBox->value();
    float phase = ui->phaseSpinBox->value();
    float attackLevel = 0;
    unsigned attackDelay = 0;
    float fadeLevel = 0;
    unsigned fadeDelay = 0;
    feature8123->downloadEffectPeriodic(&m_effectId, m_effectType, devio::IFeature8123ForceFeedback::PrimaryAxis, startNow,
                                        duration, startDelay, level, offset, waveLength, phase,
                                        attackLevel, attackDelay, fadeLevel, fadeDelay);
    ui->idLabel->setText(QString::number(m_effectId));
    refreshButtons();
}
