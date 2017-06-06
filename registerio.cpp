#include <QTextEdit>
#include <sstream>
#include <iomanip>
#include <map>
#include <string>
#include "stringbuilder.h"

#include "registerio.h"
#include "register.h"

#include "r00.h"
#include "r01.h"
#include "r02.h"
#include "r04.h"
#include "xlog/xlog.h"

RegisterIO::RegisterIO(Register* baseRegister) :
    device(baseRegister->device()),
    regnum(baseRegister->regnum())
{
}

bool RegisterIO::reg_write(const vector<Byte>& p, vector<Byte>& r)
{
    return reg_send(0x80, p, r);
}

bool RegisterIO::reg_read(vector<Byte>& r)
{
    return reg_read(vector<Byte>(), r);
}

bool RegisterIO::reg_read(const vector<Byte>& p, vector<Byte>& r)
{
    return reg_send(0x81, p, r);
}

bool RegisterIO::reg_write_long(const vector<Byte>& p, vector<Byte>& r)
{
    return reg_send(0x82, p, r);
}

bool RegisterIO::reg_read_long(vector<Byte>& r)
{
    return reg_read_long(vector<Byte>(), r);
}

bool RegisterIO::reg_read_long(const vector<Byte>& p, vector<Byte>& r)
{
    return reg_send(0x83, p, r);
}

bool RegisterIO::reg_send(int op, const vector<Byte>& p, vector<Byte>& r)
{
    vector<Byte> cmd;
    
    cmd.push_back(0xFF);
    cmd.push_back(0xFF);
    cmd.push_back(op);
    cmd.push_back(regnum);

    for (size_t i=0; i < p.size(); i++)
    {
        cmd.push_back(p[i]);
    }
    
    vector<Byte> response;
    bool result = device->sendSynchronousCommand(cmd, response, devio::Cached::No);

    r.clear();
    for (size_t i=4; i < response.size(); i++)
    {
        r.push_back(response[i]);
    }

    return result;
}

bool RegisterIO::monitor_reports()
{
    return Subscriber<devio::IDevice::Report>::subscribe(device);
}

void RegisterIO::onReport(const vector<Byte>&data, unsigned int sys, unsigned int count, devio::IDevice::ReportType)
{
	if (data.size() >= 7 && data[1] == 0xFF && data[2] >= 0x80 && data[2] <= 0x83 && data[3] == regnum)
    {
        vector<Byte> payload;
        for (size_t i = 4; i < data.size(); i++)
        {
            payload.push_back(data[i]);
        }
        onRegisterChange((data[2] == 0x80) || (data[2] == 0x82), payload);
    }
}

void RegisterIO::onRegisterChange(bool write, const vector<Byte>&data)
{
    // register UI can override this to get register change reports
}


