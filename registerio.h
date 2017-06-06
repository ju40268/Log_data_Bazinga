#ifndef REGISTERIO_H
#define REGISTERIO_H

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

class Register;

class RegisterIO :
    public Subscriber<devio::IDevice::Report>
{
public:
    explicit RegisterIO(Register* baseRegister);

    bool reg_write(const vector<Byte>& p, vector<Byte>& r);
    bool reg_read(vector<Byte>& r);
    bool reg_read(const vector<Byte>& p, vector<Byte>& r);
    bool reg_write_long(const vector<Byte>& p, vector<Byte>& r);
    bool reg_read_long(vector<Byte>& r);
    bool reg_read_long(const vector<Byte>& p, vector<Byte>& r);
    
    bool monitor_reports();
    
    void onReport(const vector<Byte>&data, unsigned int sys, unsigned int count, devio::IDevice::ReportType) override;

    virtual void onRegisterChange(bool write, const vector<Byte>&data);

private:
    bool reg_send(int op, const vector<Byte>& p, vector<Byte>& r);
    
    shared_ptr<devio::IDevice> device;
    int regnum;

signals:
    
};

#endif // REGISTERIO_H
