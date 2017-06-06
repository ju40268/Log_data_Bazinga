#ifndef F8123_FORCEWIDGET_H
#define F8123_FORCEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include "devio.h"

class f8123_ForceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit f8123_ForceWidget(shared_ptr<devio::IFeature8123ForceFeedback> f,
                               devio::IFeature8123ForceFeedback::EffectType effectType,
                               QWidget *parent = 0);

    void setPlayStopPauseButtons(QPushButton *playButton,
                                 QPushButton *stopButton,
                                 QPushButton *pauseButton);
    virtual void refreshButtons(void);
    virtual void setForceName(const QString &forceName) = 0;

signals:
    void effectDestroyed(void);

public slots:
    void onPlay(void);
    void onStop(void);
    void onPause(void);
    void onDestroyEffect(void);

protected:
    shared_ptr<devio::IFeature8123ForceFeedback> feature8123;
    unsigned m_effectId;
    devio::IFeature8123ForceFeedback::EffectType m_effectType;
    QPushButton *m_playButton;
    QPushButton *m_stopButton;
    QPushButton *m_pauseButton;
};

#endif // F8123_FORCEWIDGET_H
