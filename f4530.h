#pragma once

#include <QTextEdit>
#include "feature.h"
#include "devio.h"

using devio::Subscriber;

using std::string;


using devio::IFeature;
using devio::IFeature4530DualPlatform;

class f4530 : public QTextEdit,
public Subscriber<devio::IFeature4530DualPlatform::Report>
{
    Q_OBJECT
public:
    explicit f4530(shared_ptr<devio::IFeature4530DualPlatform> f, Feature* base);

public:
// IFeature4530DualPlatform::Report
    virtual void onPlatformChangeEvent(short newPlatformSetting) override;

protected:
    void logPlatform(short platform);
    
private:
    shared_ptr<devio::IFeature4530DualPlatform> feature4530;
    Feature* baseFeature;
	std::string desc_dualplatform( short platform);
};
