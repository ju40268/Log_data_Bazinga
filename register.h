#ifndef REGISTER_H
#define REGISTER_H

#include <QListWidgetItem>
#include <QStackedLayout>
#include <QLabel>
#include <QString>
#include "devio.h"
#include "device.h"
#include "featurebase.h"

using devio::Byte;
using std::vector;
using devio::IDevice;
using devio::Subscriber;

class Register :
    public FeatureBase,
    public Subscriber<devio::IDevice::Report>
{
    Q_OBJECT
public:
    explicit Register(Device *dev, QStackedLayout *sl, QLabel* errlabel, shared_ptr<devio::IDevice> d, int regnum_);
    void activate() override;

    shared_ptr<devio::IDevice> device() {return device_;}
    int regnum() {return regnum_;}

private:
    shared_ptr<devio::IDevice> device_;
    int regnum_;

public:
    static string register_name(int number);
    
signals:
    
};

#endif // REGISTER_H
