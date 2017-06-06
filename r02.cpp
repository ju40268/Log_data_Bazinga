#include "feature.h"
#include "r02.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include "util.h"
#include "register.h"

using std::string;
using std::vector;
using devio::Byte;

r02::r02(Register* baseRegister) :
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
           << "Read Short Register 02 = ("
           << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[0]
           << " " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[1]
           << " " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[2]
           << ") -> " << describe_err(ok)
           );

    if (ok)
    {
        append(stringbuilder()
               << "Number of Connected Devices:  "
               << (unsigned int)r[1]
               );
               
        append(stringbuilder()
               << "Number of Remaining Pairing Slots:  "
               << (unsigned int)r[2]
               );
        
        append("");
    }
    
    monitor_reports();
}

void r02::onRegisterChange(bool write, const vector<Byte>&r)
{
    if (write)
    {
        append("Register written to");
    }
    else
    {
        append("Register read");

        append(stringbuilder()
               << "Number of Connected Devices:  "
               << (unsigned int)r[1]
               );
        
        append(stringbuilder()
               << "Number of Remaining Pairing Slots:  "
               << (unsigned int)r[2]
               );
    }
}

