#pragma once

#include <QTextEdit>
#include "feature.h"
#include "devio.h"

using devio::IFeature;
using devio::IFeature4540KBLayout;

class f4540 : public QTextEdit
{
    Q_OBJECT
public:
    explicit f4540(shared_ptr<devio::IFeature4540KBLayout> f, Feature* base);
private:
    shared_ptr<devio::IFeature4540KBLayout> feature4540;
    Feature* baseFeature;
};
