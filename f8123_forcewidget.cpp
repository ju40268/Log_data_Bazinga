#include "f8123_forcewidget.h"

f8123_ForceWidget::f8123_ForceWidget(shared_ptr<devio::IFeature8123ForceFeedback> f,
                                     devio::IFeature8123ForceFeedback::EffectType effectType,
                                     QWidget *parent) :
    QWidget(parent),
    feature8123(f),
    m_effectId(0),
    m_effectType(effectType),
    m_playButton(NULL),
    m_stopButton(NULL),
    m_pauseButton(NULL)
{
}

void f8123_ForceWidget::setPlayStopPauseButtons(QPushButton *playButton,
                                                QPushButton *stopButton,
                                                QPushButton *pauseButton)
{
    m_playButton = playButton;
    m_stopButton = stopButton;
    m_pauseButton = pauseButton;

    connect(m_playButton, SIGNAL(clicked()), this, SLOT(onPlay()));
    connect(m_stopButton, SIGNAL(clicked()), this, SLOT(onStop()));
    connect(m_pauseButton, SIGNAL(clicked()), this, SLOT(onPause()));

    refreshButtons();
}

void f8123_ForceWidget::refreshButtons(void)
{
    bool enabled = (0 != m_effectId);
    m_playButton->setEnabled(enabled);
    m_stopButton->setEnabled(enabled);
    m_pauseButton->setEnabled(enabled);
}

void f8123_ForceWidget::onPlay(void)
{
    if (0 != m_effectId)
    {
        feature8123->setEffectState(m_effectId, devio::IFeature8123ForceFeedback::Play);
    }
}

void f8123_ForceWidget::onStop(void)
{
    if (0 != m_effectId)
    {
        feature8123->setEffectState(m_effectId, devio::IFeature8123ForceFeedback::Stop);
    }
}

void f8123_ForceWidget::onPause(void)
{
    if (0 != m_effectId)
    {
        feature8123->setEffectState(m_effectId, devio::IFeature8123ForceFeedback::Pause);
    }
}

void f8123_ForceWidget::onDestroyEffect(void)
{
    if (0 != m_effectId)
    {
        feature8123->destroyEffect(m_effectId);
    }
    emit effectDestroyed();
}
