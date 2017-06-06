#include "feature.h"
#include "r01.h"
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
    { 0, 0, "Mouse Sensor Resolution at Max" },
    { 0, 1, "Special Button Function Enabled" },
    { 0, 2, "Enhanced Key Usage" },
    { 0, 3, "Fast Forward and Rewind Enabled" },
    { 0, 4, "Send Calculator Result Enabled" },
    { 0, 5, "Motion Wakeup Enabled" },
    { 0, 6, "Fast Scrolling Enabled" },
    { 0, 7, "Buttons Control Resolution" },
    { 1, 1, "Receiver Multiple RF Locking" },
    { 1, 2, "Receiver Disable RF Scan in Suspend" },
    { 1, 3, "Accept All Devices During Pairing" },
    { 2, 0, "Inhibit Lock Key Sound" },
    { 2, 1, "Inhibit Touch Pad" },
    { 2, 2, "3D Engine Enabled" },
    { 2, 3, "SW Controls LEDs" },
    { 2, 4, "No Numlock Toggle" },
    { 2, 5, "Inhibit Presence Detection" },
};

r01::r01(Register* baseRegister) :
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
           << "Read Short Register 01 = ("
           << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[0]
           << " " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[1]
           << " " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[2]
           << ") -> " << describe_err(ok)
           );
    
    if (ok)
    {
        append(stringbuilder() << "Bits that are on:  " << bitlist(r, bitfields, COUNT_OF(bitfields)));
    }
    
    monitor_reports();
}

void r01::onRegisterChange(bool write, const vector<Byte>&r)
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

