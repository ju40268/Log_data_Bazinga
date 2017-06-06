#pragma once

#include <QTextEdit>
#include "feature.h"
#include "devio.h"

using devio::IFeature;
using devio::IFeature6012Gestures;

class f6012 : public QTextEdit
{
    Q_OBJECT
public:
    explicit f6012(shared_ptr<devio::IFeature6012Gestures> f, Feature* base);
private:
    shared_ptr<devio::IFeature6012Gestures> feature6012;
    Feature* baseFeature;
};
