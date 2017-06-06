#pragma once

#include <QTextEdit>
#include "feature.h"
#include "devio.h"
#include "devio_features.h"

using devio::IFeature;
using devio::IFeature6010Gestures;

class f6010 : public QTextEdit
{
    Q_OBJECT
public:
    explicit f6010(shared_ptr<devio::IFeature6010Gestures> f, Feature* base);
private:
    shared_ptr<devio::IFeature6010Gestures> feature6010;
    Feature* baseFeature;
};
