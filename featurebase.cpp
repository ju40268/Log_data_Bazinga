#include "feature.h"
#include <QTextEdit>
#include <sstream>
#include <iomanip>
#include <map>
#include <string>
#include "stringbuilder.h"

#include "xlog/xlog.h"
#include "devio.h"

using std::string;
using devio::IDevice;

string feature_name(int number);

FeatureBase::FeatureBase(Device *dev, QStackedLayout *sl, QLabel* errlabel) :
    QListWidgetItem(),
    device(dev),
    errlabel(errlabel),
    sl(sl),
    ui(nullptr)
{
    std::ostringstream s;
    s << "FeatureBase";
    setText(s.str().c_str());
}

FeatureBase::~FeatureBase()
{
    delete ui;
}

void FeatureBase::activate()
{
    if (!ui)
    {
        stringbuilder s;
        s << "FeatureBase\n";

        description = s;

        QTextEdit *te = new QTextEdit();
        te->setPlainText(description);
        ui = te;

        index = sl->addWidget(ui);
    }

    // need to know when ui object was deleted and then set its pointer to NULL, otherwise we will
    // double delete it (no double dipping, ur, deleting please)
    QObject::connect(ui, SIGNAL(destroyed(QObject*)), SLOT(onUIDestroyed(QObject*)));

    sl->setCurrentIndex(index);
}

void FeatureBase::onUIDestroyed(QObject *obj /*= NULL*/)
{
    if(obj == ui)
    {
        ui = NULL;
    }
}

void FeatureBase::handleConfigChange(shared_ptr<IDevice> dev)
{
    device->handleConfigChange(dev);
}
