#include "feature.h"
#include <QPlainTextEdit>
#include <QMenu>
#include <QMessageBox>
#include <sstream>
#include <iomanip>
#include <map>
#include <string>
#include <chrono>
#include "stringbuilder.h"
#include "features_all.h"
#include "devio.h"
#include "Device.h"
#include <QDateTime>
#include <fstream>
// newly added 
#include <iostream>
using namespace std;
//adding namespace std
using std::string;
using std::vector;
using devio::Byte;
using devio::BusType;

FeaturesAll::FeaturesAll(Device *dev, QStackedLayout *sl, shared_ptr<devio::IDevice> d, QLabel* errlabel) :
    FeatureBase(dev,sl,errlabel),
    device(d),
    te(nullptr)
{
    std::ostringstream s;
    s << "HID++";
    setText(s.str().c_str());
    qRegisterMetaType<IDevice::ReportType>("IDevice::ReportType");
    qRegisterMetaType<vector<Byte>>("vector<Byte>");

    bool connectedRead = connect(this, SIGNAL(signalRead(unsigned int, const vector<Byte>, unsigned int, unsigned int, IDevice::ReportType)),
                             this, SLOT(slotRead(unsigned int, const vector<Byte>, unsigned int, unsigned int, IDevice::ReportType)),
                             Qt::QueuedConnection);

    bool connectedWrite = connect(this, SIGNAL(signalWrite(unsigned int, const vector<Byte>, ErrorCode, unsigned int)),
                             this, SLOT(slotWrite(unsigned int, const vector<Byte>, ErrorCode, unsigned int)),
                             Qt::QueuedConnection);

    activate();             // start logging before used clicks on anything

    if (!connectedRead)
    {
        te->appendPlainText("Error: signalRead not connected to slotRead!");
    }

    if (!connectedWrite)
    {
        te->appendPlainText("Error: signalWrite not connected to slotWrite!");
    }
}
void FeaturesAll::writeFile(string s){
	std::ofstream outfile;
	outfile.open("Output.txt", std::ios_base::app);
	outfile << s;
}
void FeaturesAll::activate()
{
    if (!ui)
    {
        stringbuilder s;
        s << "HID++ traffic ";

        switch (device->bus_type())
        {
        case BusType::USB:
            s << "over USB";
            break;
        case BusType::Unifying:
            s << "over Unifying (Gaudi)";
            break;
        case BusType::Bluetooth:
            s << "over Bluetooth";
            break;
        case BusType::BLE:

            if (device->bus_port() == 0)
            {
                s << "over BLE";
            }

            if (device->bus_port() == 1)
            {
                s << "over blehidpp";
            }

            if (device->bus_port() == 2)
            {
                s << "over blepp";
            }

            break;
        default:
            s << "over unknown bus";
            break;
        }

        description = s;

        {
            te = new QPlainTextEdit();
            te->setPlainText(description);

//            QFont fnt("",10);
//            fnt.setStyleHint(QFont::Monospace);
//           te->setCurrentFont(fnt);

            te->setReadOnly(true);
            te->setContextMenuPolicy(Qt::CustomContextMenu);
            te->setWordWrapMode(QTextOption::NoWrap);
            connect(te, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenu(const QPoint&)));
        }

        if (!Subscriber<IDevice::Report>::subscribe(device, devio::Delivery::Immediate))
        {
            te->appendPlainText("failed to subscribe to reads");
        }

        if (!Subscriber<IDevice::WriteReport>::subscribe(device, devio::Delivery::Immediate))
        {
            stringbuilder s;
            s << "failed to subscribe to writes, err=" << devio::errcode;
            te->appendPlainText(s);
        }

        if (!Subscriber<IDevice::ConfigChange>::subscribe(device))
        {
            te->appendPlainText("failed to subscribe to config change");
        }

        te->appendPlainText("");

        auto rcv = dynamic_pointer_cast<devio::IReceiver>(device);
        user_command = new QPlainTextEdit();
		//
		// here the default plain text
		//
        user_command->setPlainText(rcv ? "1X " : "1X XX ");
        user_command->setFixedHeight(25);
        user_command->setWordWrapMode(QTextOption::NoWrap);
        user_command->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        user_command->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		/*
        send_button = new QPushButton();
        send_button->setText("Send signal !!! (Gaudi)");
        connect(send_button, SIGNAL(pressed()), this, SLOT(doSend()));
        connect(user_command, SIGNAL(textChanged()), this, SLOT(validateCommandToSend()));*/ 

		//new button

		send_button = new QPushButton();
		send_button->setText("Record Data(Gaudi)");
		connect(send_button, SIGNAL(pressed()), this, SLOT(recordData()));
		connect(user_command, SIGNAL(textChanged()), this, SLOT(validateCommandToSend()));

		openlock_button = new QPushButton();
		openlock_button->setText("Open Lock");
		connect(openlock_button, SIGNAL(pressed()), this, SLOT(doOpenLock()));

        closelock_button = new QPushButton();
        closelock_button->setText("Close Lock");
        connect(closelock_button, SIGNAL(pressed()), this, SLOT(doCloseLock()));

        unpair_button = new QPushButton();
        unpair_button->setText("Unpair");
        connect(unpair_button, SIGNAL(pressed()), this, SLOT(doUnpair()));

        QHBoxLayout *hbox = new QHBoxLayout();
        hbox->addWidget(user_command,1);
        hbox->addWidget(send_button,0);

        if (std::dynamic_pointer_cast<devio::IReceiver>(device))
        {
            // this is a receiver, so expose open lock and close lock buttons

            hbox->addWidget(openlock_button, 0);
            hbox->addWidget(closelock_button, 0);
            hbox->addWidget(unpair_button, 0);

        }
        else if (device->receiver())
        {
            // this is connected to a receiver, so expose unpair button
            hbox->addWidget(unpair_button, 0);
        }

        QVBoxLayout *vbox = new QVBoxLayout();
        vbox->addWidget(te,1);
        vbox->addLayout(hbox,0);
        vbox->setContentsMargins(0,0,0,0);

        ui = new QWidget();
        ui->setLayout(vbox);

        index = sl->addWidget(ui);
    }

    FeatureBase::activate();

    validateCommandToSend();
}


void FeaturesAll::onContextMenu(const QPoint &pos)
{
    (void) pos;
    QMenu *menu = new QMenu;
    menu->addAction(QString("Clear"), this, SLOT(onClear()));
    menu->exec(QCursor::pos());
}

// validate that a value is present, consists of valid hex chars (may have 'X' wildcards in first 2 bytes), and digits appear in pairs (except last digit so button doesn't flash as you type)
void FeaturesAll::validateCommandToSend()
{
    QString str = user_command->toPlainText();

    bool ok = true;
    int digits = 0;
    bool incomplete = false;  // only one digit of a byte has been parsed

    for (int i=0; i < str.size() && ok; i++)
    {
        if (str[i] == ' ')
        {
            // optional spaces may separate complete 2-digit hex values
            if (incomplete)
            {
                ok = false;
            }
        }
        else
        {
            QChar c = str[i];
            if (digits < 4 && (c == 'x' || c == 'X'))
            {
                // allow "X" wildcards in first 2 bytes
            }
            else
            {
                // all others must be hex digits
                QString(c).toUInt(&ok, 16);
            }

            incomplete = !incomplete;
            digits++;
        }
    }

    if (digits == 0)
    {
        ok = false;
    }

    send_button->setEnabled(ok);
}

void FeaturesAll::doOpenLock(void)
{

	auto rcv = dynamic_pointer_cast<devio::IReceiver>(device);
	if (rcv)
	{
		rcv->open_lock();
	}
}


void FeaturesAll::doCloseLock(void)
{

    auto rcv = dynamic_pointer_cast<devio::IReceiver>(device);
    if (rcv)
    {
        rcv->close_lock();
    }
}

void FeaturesAll::doUnpair(void)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(ui, "", QString("Unpairing %1").arg(device->name().c_str()), QMessageBox::Ok|QMessageBox::Cancel);
    if (reply != QMessageBox::Ok)
    {
        return;
    }
    if (std::dynamic_pointer_cast<devio::IReceiver>(device))
    {
        auto rcv = std::dynamic_pointer_cast<devio::IReceiver>(device);
        if (rcv)
        {
            rcv->unpair(1);
        }
    }
    else
    {
        auto rcv = device->receiver();
        if (rcv)
        {
            rcv->unpair(device->bus_port());
        }
    }
    
}


// record the data
void FeaturesAll::recordData(void)
{
	/*
	QString str = user_command->toPlainText();
	//replace all the command with X/x to '0'
	str.replace('x', '0');
	str.replace('X', '0');
	QByteArray qba = QByteArray::fromHex(str.toLatin1());

	//printing out the command 10 00 or 11 0 0A 0F 10/30/31
	OutputDebugStringW(L"testing output");
	std::string s = str.toStdString();
	std::wstring stemp = std::wstring(s.begin(), s.end());
	LPCWSTR sw = stemp.c_str();
	OutputDebugStringW(sw);
	OutputDebugStringW(L"\n");

	 

	char *data = qba.data();
	int size = qba.size();

	vector<Byte> command(data, data + size);
	vector<Byte> response;
	// printing out command
	/*
	OutputDebugStringW(L"command");
	std::string scommand(reinterpret_cast<const char *>(&qba[0]), qba.length());
	std::wstring wscommand = std::wstring(scommand.begin(), scommand.end());
	LPCWSTR swtempcommand = wscommand.c_str();
	OutputDebugStringW(swtempcommand);
	OutputDebugStringW(L"\n");*/

	//device->sendCommand(command, false);
	

}
void FeaturesAll::onClear(void)
{
    if (te)
    {
        te->clear();
    }
}


void FeaturesAll::onReport(const vector<Byte>&v, unsigned int sys, unsigned int count, IDevice::ReportType rt)
{
    emit signalRead(int_timestamp(), v, sys, count, rt);
}

void FeaturesAll::onWriteReport(const vector<Byte>&v, ErrorCode err, unsigned int sys)
{
    emit signalWrite(int_timestamp(), v,err,sys);
}

void FeaturesAll::onConfigChange()
{
    handleConfigChange(device);
}


// here!!!! marked for the output
void FeaturesAll::slotRead(unsigned int ts, const vector<Byte>v, unsigned int sys, unsigned int count, IDevice::ReportType rt)
{
    stringbuilder s;
	stringbuilder outputString;
	//record the file output 
	//switch type
	switch (rt)
    {
    case IDevice::ReportType::Ignored:
        s << "<font face='courier' size='2' color='Gray'>";
        break;
    case IDevice::ReportType::Notification:
        s << "<font face='courier' size='2' color='Green'>";
        break;
    case IDevice::ReportType::UnexpectedResponse:
        s << "<font face='courier' size='2' color='Blue'>";
        break;
    default:
    case IDevice::ReportType::Response:
        s << "<font face='courier' size='2'>";
        break;
    }
    s << timestamp(ts);
	s << " R: ";
	outputString << timestamp(ts);
	outputString << " R: ";
    for (auto val : v)
    {
		if (count-- == 0)
        {
            break;
        }
		s << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)val << " ";
		outputString << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)val << " ";
    }
	outputString << "\n";
	writeFile(outputString);
    if (sys)
    {
        s << "err=" << sys;
    }
    s << "</font'>";
    if (te)
    {
        te->appendHtml(s);
    }
}

void FeaturesAll::slotWrite(unsigned int ts, const vector<Byte>v, ErrorCode err, unsigned int sys)
{
    stringbuilder s;
	//stringbuilder outputString;
    s << "<font face='courier' size='2' color='Red'>";
    //s << timestamp(ts);
	//outputString << timestamp(ts);
    s << " W: ";
	//outputString << " W: ";
	/*
	for (auto val : v)
    {
       // s << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)val << " ";
		//outputString << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)val << " ";
    }
	*/
	//outputString << "\n";
	//writeFile(outputString);
	/*
    if (err != ErrorCode::Success)
    {
        s << std::dec << "err=" << err;
        if (sys)
        {
            s << "," << sys;
        }
    }*/

    s << "</font'>";
	/*
	if (te)
    {
        te->appendHtml(s);
    }*/
}

string FeaturesAll::timestamp(unsigned int int_timestamp)
{
    unsigned int sec = int_timestamp / 1000;
    unsigned int mils = int_timestamp - sec * 1000;

    sec = sec % 60;

    stringbuilder s;
    QDateTime current = QDateTime::currentDateTime();
    s << std::setfill('0') << std::setw(2) << sec << "." << std::setfill('0') << std::setw(3) << mils << " t:" <<current.toTime_t();
    return s;
}

string FeaturesAll::timestamp_full(unsigned int int_timestamp)
{
    unsigned int sec = int_timestamp / 1000;
    unsigned int mils = int_timestamp - sec * 1000;

    unsigned int minutes = (sec / 60) % 60;

    sec = sec % 60;

    stringbuilder s;
    s << std::setfill('0') << std::setw(2) << minutes << ":" 
      << std::setfill('0') << std::setw(2) << sec << "." 
      << std::setfill('0') << std::setw(3) << mils;

    return s;
}

unsigned int FeaturesAll::int_timestamp()
{
    using namespace std::chrono;

    auto duration = high_resolution_clock::now().time_since_epoch();
    auto milli = (unsigned int)duration_cast<std::chrono::milliseconds>(duration).count();

    return milli;
}
