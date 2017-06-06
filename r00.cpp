#include "feature.h"
#include "r00.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include "util.h"
#include "register.h"

using std::string;
using std::vector;
using devio::Byte;

static struct s_named_bitfield bitfields[] =
{
    { 0, 0, "Consumer & Vendor Specific Control" },
    { 0, 1, "Power Keys" },
    { 0, 2, "Roller V" },
    { 0, 3, "Mouse Extra Buttons" },
    { 0, 4, "Battery Status" },
    { 0, 5, "Roller H" },
    { 0, 6, "F-Lock Status" },
    { 0, 7, "Numpad Numeric Keys" },
    { 1, 0, "Wireless Notifications" },
    { 1, 1, "UI Notifications" },
    { 1, 2, "Quad link quality info" },
    { 1, 3, "Software Present" },
    { 1, 4, "Touchpad multi-touch notifications" },
    { 2, 0, "3D Gesture" },
    { 2, 1, "VoIP Telephony" },
    { 2, 2, "Configuration Complete" },
};

r00::r00(Register* baseRegister) :
    QTextEdit(),
    RegisterIO(baseRegister),
    base(baseRegister)
{
    setPlainText(baseRegister->description);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);

    vector<Byte> r;
    bool ok = reg_read(r);
    r.resize(3);

    append( stringbuilder()
            << "Read Short Register 00 = ("
            << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[0]
            << " " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[1]
            << " " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[2]
            << ") -> " << describe_err(ok)
            );

    if (ok)
    {
        append(stringbuilder() << "Bits that are on:  " << bitlist(r, bitfields, COUNT_OF(bitfields)));
    }

    append("");
    
    monitor_reports();
}

void r00::onRegisterChange(bool write, const vector<Byte>&r)
{
    if (write)
    {
        append("Register written to... we will read it to get new value");

        // value is not echoed, so need to read it to see what it was changed to
        vector<Byte> read;
        reg_read(read);
    }
    else
    {
        append("Register read");
        append(stringbuilder() << "Bits that are on:  " << bitlist(r, bitfields, COUNT_OF(bitfields)));
    }
}
