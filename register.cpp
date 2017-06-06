#include <QTextEdit>
#include <sstream>
#include <iomanip>
#include <map>
#include <string>
#include "stringbuilder.h"

#include "register.h"

#include "r00.h"
#include "r01.h"
#include "r02.h"
#include "r04.h"
#include "xlog/xlog.h"

using std::string;

string register_name(int number);

Register::Register(Device *dev, QStackedLayout *sl, QLabel* errlabel,
                   shared_ptr<devio::IDevice> device, int regnum) :
    FeatureBase(dev,sl,errlabel),
    device_(device),
    regnum_(regnum)
{
    std::ostringstream s;
    s << std::hex << std::setfill('0') << std::setw(2) << regnum << " - " << register_name(regnum);
    setText(s.str().c_str());
}


void Register::activate()
{
    if (!ui)
    {
        stringbuilder s;
        s << "Register " << std::hex << regnum_;

        s << " - " << register_name(regnum_) << "  ";
        s << "\n";

        description = s;

        if (regnum_ == 0x00)
        {
            ui = new r00(this);
        }
        else if (regnum_ == 0x01)
        {
            ui = new r01(this);
        }
        else if (regnum_ == 0x02)
        {
            ui = new r02(this);
        }
       else if (regnum_ == 0xb4)
        {
            ui = new r04(this);
        }
        else
        {
            QTextEdit *te = new QTextEdit();
            te->setPlainText(description);
            ui = te;
        }

        // need to know when ui object was deleted and then set its pointer to NULL, otherwise we will
        // double delete it (no double dipping, ur, deleting please)
        QObject::connect(ui, SIGNAL(destroyed(QObject*)), SLOT(onUIDestroyed(QObject*)));

        index = sl->addWidget(ui);
    }

    FeatureBase::activate();
}

string Register::register_name(int number)
{
    using std::map;
    static map<int,string> register_names;

    if (register_names.empty())
    {
        register_names[0x0000] = "HID++ Reporting";
        register_names[0x0001] = "Individual Features";
        register_names[0x0002] = "Connection State";
        register_names[0x00b4] = "Get RSSI";

    }

    return register_names[number];
}

