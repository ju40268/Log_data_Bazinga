#include "feature.h"
#include "f4530.h"
#include <sstream>
#include <iomanip>
#include <QtDebug>
#include "stringbuilder.h"
using devio::Delivery;


f4530::f4530(shared_ptr<devio::IFeature4530DualPlatform> f, Feature* base) :
    QTextEdit(),
    feature4530(f),
    baseFeature(base)
{
    setPlainText(baseFeature->description);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);
	
	short platform =0;
    bool ok = feature4530->GetPlatform(&platform);

    logPlatform(platform);
    append (stringbuilder() << " -> " << describe_err(ok));
    
    bool bresult = subscribe(feature4530, Delivery::Immediate);
    append(stringbuilder()
           << "Subscribing to 4530 reports -> "
           << describe_err(bresult));
}

std::string f4530::desc_dualplatform( short platform )
{
	switch (platform )
	{
		case 0 :
            return "iOS or Mac OSX";
            break;
		case 1 :
            return "Android or Windows";
            break;
	}
    
	return "unknown platform ";
}

void f4530::onPlatformChangeEvent(short newPlatformSetting)
{
    append (stringbuilder()
            << "onPlatformChangeEvent(newPlatformSetting = "
            << newPlatformSetting
            << ")");
}

void f4530::logPlatform(short platform)
{
	append( stringbuilder()
           << "("
           << " GetPlatform=" << platform
           <<  "( "
           << desc_dualplatform(platform)
           << ") )" );
}
