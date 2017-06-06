#ifndef F6100_H
#define F6100_H

#include "feature.h"
#include <QTabWidget>
#include <QTextEdit>
#include "devio.h"
#include <QRadioButton>
#include <vector>

using devio::IFeature;
using devio::IFeature6100TouchPadRawXY;
using devio::Subscriber;
using std::vector;

class f6100 : public QTabWidget,
        public Subscriber<devio::IFeature6100TouchPadRawXY::Report>
{
    Q_OBJECT
public:
    explicit f6100(shared_ptr<devio::IFeature6100TouchPadRawXY> f, Feature* base);
    void onDualXYData(const IFeature6100TouchPadRawXY::DualXYData data)override;
    void onModeChanged(unsigned int swid)override;
    void settings_lost();
private:
    shared_ptr<devio::IFeature6100TouchPadRawXY> feature;
    Feature* baseFeature;
    int mode;
    QTextEdit *text;
    QWidget *controls;
    vector<QRadioButton *>radio_buttons;

signals:
    void signalDualXYData(const IFeature6100TouchPadRawXY::DualXYData data);
    void signalModeChanged(unsigned int swid);

private slots:
    void slotDualXYData(const IFeature6100TouchPadRawXY::DualXYData data);
    void slotModeChanged(unsigned int swid);
    void rbutton_clicked(bool click);

};

#endif // F6100_H
