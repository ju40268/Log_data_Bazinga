#pragma once

#include <QTextEdit>
#include "feature.h"
#include <QTabWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QListWidget>
#include "devio.h"
#include "devio_features.h"

using devio::IFeature;
using devio::IFeature4600Crown;
using devio::Subscriber;

class f4600 : public QTabWidget,
        public Subscriber<devio::IFeature4600Crown::CrownEvent>
{
    Q_OBJECT
public:
    explicit f4600(shared_ptr<devio::IFeature4600Crown> f, Feature* base);
    void settings_lost();
private:
    shared_ptr<devio::IFeature4600Crown> feature4600;
    struct IFeature4600Crown::CrownInfo _info{};
    Feature* baseFeature;
    QTextEdit *log;
    QTextEdit* notifications;
    QTextEdit* capabilities;
    QListWidget *page_state;

    QButtonGroup* reportingGroup;
    QRadioButton* reportingHid;
    QRadioButton* reportingDiverted;

    QButtonGroup* modeGroup;
    QRadioButton* modeFreewheel;
    QRadioButton* modeRatchet;

    QLabel* rotationTimeoutValue;
    QLineEdit* rotationTimeoutEdit;
    QPushButton* rotationTimeoutSet;

    QLabel* shortLongTimeoutValue;
    QLineEdit* shortLongTimeoutEdit;
    QPushButton* shortLongTimeoutSet;

    QLabel* doubleTapSpeedValue;
    QLineEdit* doubleTapSpeedEdit;
    QPushButton* doubleTapSpeedSet;

    void createUI();

    void updateModeDisplay(struct IFeature4600Crown::CrownMode mode);

    void readInfo();
    void readMode();
    void setReportingDiverted(unsigned int reportingDiverted);
    void setMode(unsigned int ratchetMode);
    void setRotationTimeout(unsigned int rotationTimeout);
    void setShortLongTimeout(unsigned int shortLongTimeout);
    void setDoubleTapSpeed(unsigned int doubleTapSpeed);

    void capabilitiesOutput(const QString &text);
    void notifyText(const QString &text, bool insertCRLF = false);

    string timestamp(unsigned int int_timestamp);
    unsigned int int_timestamp();

    // IFeature4600Crown::CrownEvent
    void onCrownEvent(struct IFeature4600Crown::CrownEventData data);
    void onModeChanged(unsigned int swid, struct IFeature4600Crown::CrownMode mode);
    void SendKeyEvent(int keycode);

private slots:
    void onReportingGroupClicked(int id);
    void onModeGroupClicked(int id);
    void onClickedSetRotationTimeout();
    void onClickedShortLongTimeout();
    void onClickedSetDoubleTapSpeed();

protected slots:
    void onNotificationContextMenu(const QPoint &pos);
    void onClearNotifications();

signals:
};

