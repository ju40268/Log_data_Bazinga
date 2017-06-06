#ifndef F6501_H
#define F6501_H

#include "feature.h"
#include <QTabWidget>
#include <QTextEdit>
#include <QListWidget>
#include "devio.h"
#include <QRadioButton>
#include <QTreeWidget>
#include <QLineEdit>
#include <QPushButton>

using devio::IFeature;
using devio::IFeature6501Gestures;
using devio::Subscriber;
using std::vector;

class f6501 : public QTabWidget,
        public Subscriber<devio::IFeature6501Gestures::GestureReport>
{
    Q_OBJECT
public:
    explicit f6501(shared_ptr<IFeature6501Gestures> f, Feature* base);
    void OnGesture              (unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type) override;
    void OnGestureTriggerXY     (unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type, int x, int y) override;
    void OnGestureDxDy          (unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type, int period, int dx, int dy, int dist, int angle) override;
    void OnGestureFingerStart   (unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type, int x, int y, int w, int h) override;
    void OnGestureFingerProgress(unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type, int period, int dX, int dY, int dW, int dH) override;
    void OnGestureUnknown       (unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type, vector<devio::Byte> v) override;
    void OnGestureEnabled       (unsigned int swid, unsigned int gi, bool enabled) override;
    void OnGestureDiverted      (unsigned int swid, unsigned int gi, bool diverted) override;
    void OnParamChange          (unsigned int swid, unsigned int pi, std::vector<devio::Byte> new_value) override;
    void settings_lost();
private:
    shared_ptr<devio::IFeature6501Gestures> feature6501;
    Feature* baseFeature;
    QTextEdit *log;
    QTreeWidget *page_descriptor;
    QTreeWidget *page_gesture;
    QListWidget *page_enable;
    QListWidget *page_divert;
    QTreeWidget *page_spec;
    QWidget *page_param;
    QTreeWidget *page_param_list;
    QLineEdit *page_param_text;
    QPushButton *page_param_get;
    QPushButton *page_param_set;
    QPushButton *page_param_def;
    QRadioButton **rbuttons;
    vector<IFeature6501Gestures::GestureInfo> gestures;
    string gestureName(IFeature6501Gestures::GestureId gesture);
    string specName(IFeature6501Gestures::SpecId spec);
    string paramName(IFeature6501Gestures::ParamId spec);
    vector<unsigned int>divert_list;
    vector<unsigned int>enable_list;
    void readDivertedState(unsigned int gesture_index, QListWidgetItem*item);
    void readEnabledState(unsigned int gesture_index, QListWidgetItem*item);
    void updateButtons();
    map<unsigned int,unsigned int> param_lengths;
    map<unsigned int,vector<devio::Byte>> param_defaults;
    map<unsigned int,devio::IFeature6501Gestures::ParamId> param_ids;
    void setParamText( int param_id, vector<devio::Byte> value);

    string timestamp(unsigned int int_timestamp);
    unsigned int int_timestamp();

signals:
    void signalGestureReport(const QString txt);
    void signalOnGestureEnabled(unsigned int swid, unsigned int gi, bool enabled);
    void signalOnGestureDiverted(unsigned int swid, unsigned int gi, bool enabled);

private slots:
    void slotGestureReport(const QString txt);
    void onEnableChanged(QListWidgetItem*);
    void onDivertChanged(QListWidgetItem*);
    void slotOnGestureEnabled(unsigned int swid, unsigned int gi, bool enabled);
    void slotOnGestureDiverted(unsigned int swid, unsigned int gi, bool diverted);
    void contextualMenu(const QPoint& point);
    void slotClear();
    void onParamTextChanged(const QString &);
    void onParamSelected();
    void onClickGet();
    void onClickSet();
    void onClickDef();
};

#endif // F6501_H
