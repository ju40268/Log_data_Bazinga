#pragma once

#include <QWidget>
#include <QSlider>
#include "feature.h"
#include "devio.h"
#include <vector>

using devio::IFeature;
using devio::IFeature8310Equalizer;
using devio::Byte;
using std::vector;


class f8310 : public QWidget
{
    Q_OBJECT

public:
    explicit f8310(shared_ptr<devio::IFeature8310Equalizer> f, Feature *base);

private slots:
    void onGainChanged(int gain);

private:
    void createUI(void);
    void readFrequencyGains(void);

    shared_ptr<devio::IFeature8310Equalizer> feature8310;
    Feature *baseFeature;

    // UI
    QList<QSlider*> gainSliderList;
};
