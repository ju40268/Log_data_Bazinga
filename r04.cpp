#include "feature.h"
#include "r04.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include "util.h"
#include "register.h"
#include <QTimer>

using std::string;
using std::vector;
using devio::Byte;

r04::r04(Register* baseRegister) :
    QTextEdit(),
    RegisterIO(baseRegister),
    base(baseRegister)
{
    setPlainText(baseRegister->description);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);

    timer_update();
    
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_update()));
    timer->start(5000);

}

void r04::onRegisterChange(bool write, const vector<Byte>&r)
{
    if (write)
    {
        append("Register written to");
    }
    else
    {
        append("Register read");

        if (r[1] == 0)
        {
            append(stringbuilder()
                << "Status(OK):  "
                << (unsigned int)r[1]
                );
        }
        else
        {
            append(stringbuilder()
                << "Status(FAILED) error :  "
                << (unsigned int)r[1]
                );
        }


        append(stringbuilder()
            << "RSSI Signal Strenght ( 0 - 64 decimal)"
            << (unsigned int)r[2]
            );

        append("");
    }

}

void r04::timer_update()
{
    vector<Byte> r;
    vector<Byte> p;
    p.resize(1);
    p[0] = 1;
    //p[1] = 1;
    bool ok = reg_read(p, r);
    r.resize(3);



    append(stringbuilder()
        << "Read Short Register b4 = ("
        << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[0]
        << " " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[1]
        << " " << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)r[2]
        << ") -> " << describe_err(ok)
        );

    if (ok)
    {
        if (r[1] == 0)
        {
            append(stringbuilder()
                << "Status(OK):  "
                << (unsigned int)r[1]
                );
        }
        else
        {
            append(stringbuilder()
                << "Status(FAILED) error :  "
                << (unsigned int)r[1]
                );
        }


        append(stringbuilder()
            << "RSSI Signal Strenght ( 0 - 64 decimal)"
            << (unsigned int)r[2]
            );

        append("");
    }

    monitor_reports();
}

