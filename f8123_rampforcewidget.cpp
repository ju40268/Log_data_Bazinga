#include "f8123_rampforcewidget.h"
#include "ui_f8123_rampforcewidget.h"

f8123_RampForceWidget::f8123_RampForceWidget(shared_ptr<devio::IFeature8123ForceFeedback> f,
                                                     devio::IFeature8123ForceFeedback::EffectType effectType,
                                                     QWidget *parent) :
    f8123_ForceWidget(f, effectType, parent),
    ui(new Ui::f8123_RampForceWidget)
{
    ui->setupUi(this);

    setPlayStopPauseButtons(ui->playButton, ui->stopButton, ui->pauseButton);

    connect(ui->destroyEffectButton, SIGNAL(clicked()), this, SLOT(onDestroyEffect()));
    connect(ui->downloadAndStartButton, SIGNAL(clicked()), this, SLOT(onDownload()));
    connect(ui->downloadButton, SIGNAL(clicked()), this, SLOT(onDownload()));
}

f8123_RampForceWidget::~f8123_RampForceWidget(void)
{
    delete ui;
}

void f8123_RampForceWidget::refreshButtons(void)
{
    f8123_ForceWidget::refreshButtons();

    ui->downloadAndStartButton->setEnabled(0 == m_effectId);
}

void f8123_RampForceWidget::setForceName(const QString &forceName)
{
    ui->effectTypeLabel->setText(forceName);
}

void f8123_RampForceWidget::onDownload(void)
{
    bool startNow = (sender() == ui->downloadAndStartButton);
    unsigned duration = ui->durationSpinBox->value();
    unsigned startDelay = ui->startDelaySpinBox->value();
    float startLevel = ui->startLevelSpinBox->value();
    float endLevel = ui->endLevelSpinBox->value();
    /* TODO: Implement envelope handling */
    float attackLevel = 0;
    unsigned attackDelay = 0;
    float fadeLevel = 0;
    unsigned fadeDelay = 0;
    feature8123->downloadEffectRamp(&m_effectId, m_effectType, devio::IFeature8123ForceFeedback::PrimaryAxis, startNow,
                                    duration, startDelay, startLevel, endLevel,
                                    attackLevel, attackDelay, fadeLevel, fadeDelay);
    ui->idLabel->setText(QString::number(m_effectId));
    refreshButtons();
}
