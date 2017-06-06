#include "feature.h"
#include "f4600.h"
#include <sstream>
#include <iomanip>
#include "qbuttongroup.h"
#include "qgroupbox.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qmenu.h"
#include "stringbuilder.h"

#if defined __APPLE__

#include "inject.h"

#endif


// Fake cache lines used for testing the 4600 Bazinga UI with a M320 mouse:
//
//	<command cmd="10FF010A" rsp="11FF010A18000000000000000000000000000000"/>
//	<command cmd="10FF011A18" rsp="11FF011A46000000000000000000000000000000"/>
//	<command cmd="10FF180A" rsp="11FF180A1F0F0014006400000000000000000000"/>
//	<command cmd="10FF181A" rsp="11FF181A02021E02280000000000000000000000"/>   in sendSynchronousCommand, comment out "if (cached == Cached::Yes)"
//

using devio::Delivery;

f4600::f4600(shared_ptr<devio::IFeature4600Crown> f, Feature* base) :
    QTabWidget(),
    feature4600(f),
    baseFeature(base)
{
    createUI();
}


void f4600::createUI()
{
    log = new QTextEdit;
    log->setPlainText(baseFeature->description);
    log->setReadOnly(true);
    log->setWordWrapMode(QTextOption::NoWrap);

    auto vLayout = new QVBoxLayout;
    page_state = new QListWidget;

    // radio button groups
    {
        auto hLayout = new QHBoxLayout;

        // Reporting
        {
            QGroupBox* group = new QGroupBox("Reporting");
            auto currentLayout = new QVBoxLayout;
            group->setLayout(currentLayout);

            reportingGroup = new QButtonGroup;

            reportingHid = new QRadioButton("Hid", page_state);
            reportingGroup->addButton(reportingHid, (int)devio::IFeature4600Crown::Diverting::Hid);
    
            reportingDiverted = new QRadioButton("Diverted", page_state);
            reportingGroup->addButton(reportingDiverted, (int)devio::IFeature4600Crown::Diverting::Diverted);

            page_state->setLayout(vLayout);

            // monitoring of user clicking on radiobutton
            if (!connect(reportingGroup, SIGNAL(buttonClicked(int)), this, SLOT(onReportingGroupClicked(int))))
            {
                log->append("Error: Signal buttonClicked(int) to slot onModeButtonClicked(int) not connected!");
            }

            currentLayout->addWidget(reportingHid);
            currentLayout->addWidget(reportingDiverted);
            hLayout->addWidget(group);
        }

        // Mode setting
        {
            QGroupBox* group = new QGroupBox("Ratchet mode");
            auto currentLayout = new QVBoxLayout;
            group->setLayout(currentLayout);

            modeGroup = new QButtonGroup;

            modeFreewheel = new QRadioButton("Freewheel mode", page_state);
            modeGroup->addButton(modeFreewheel, (int)devio::IFeature4600Crown::RatchetMode::Freewheel);
    
            modeRatchet = new QRadioButton("Ratchet mode", page_state);
            modeGroup->addButton(modeRatchet, (int)devio::IFeature4600Crown::RatchetMode::Ratchet);

            page_state->setLayout(vLayout);

            // monitoring of user clicking on radiobutton
            if (!connect(modeGroup, SIGNAL(buttonClicked(int)), this, SLOT(onModeGroupClicked(int))))
            {
                log->append("Error: Signal buttonClicked(int) to slot onModeButtonClicked(int) not connected!");
            }

            currentLayout->addWidget(modeFreewheel);
            currentLayout->addWidget(modeRatchet);
            hLayout->addWidget(group);
        }

        vLayout->addLayout(hLayout);
    }

    // numerical parameters and capabilities
    {
        auto hLayout = new QHBoxLayout;

        // numerical parameters
        {
            auto vLayout = new QVBoxLayout;

            // Rotation Timeout
            {
                QGroupBox* group = new QGroupBox("Rotation Timeout");
                QHBoxLayout* currentLayout = new QHBoxLayout;
                group->setLayout(currentLayout);

                rotationTimeoutValue = new QLabel();

                rotationTimeoutSet = new QPushButton("<-- Set");
                rotationTimeoutEdit = new QLineEdit();
                rotationTimeoutEdit->setValidator(new QIntValidator(0,0xFF));

                connect(rotationTimeoutSet,SIGNAL(clicked(bool)),this,SLOT(onSetRotationTimeout(void)));

                currentLayout->addWidget(rotationTimeoutValue);
                currentLayout->addWidget(rotationTimeoutSet);
                currentLayout->addWidget(rotationTimeoutEdit);

                vLayout->addWidget(group);
            }

            // Short-long timeout
            {
                QGroupBox* group = new QGroupBox("Short-Long Timeout");
                QHBoxLayout* currentLayout = new QHBoxLayout;
                group->setLayout(currentLayout);

                shortLongTimeoutValue = new QLabel();

                shortLongTimeoutSet = new QPushButton("<-- Set");
                shortLongTimeoutEdit = new QLineEdit();
                shortLongTimeoutEdit->setValidator(new QIntValidator(0,0xFF));

                connect(shortLongTimeoutSet,SIGNAL(clicked(bool)),this,SLOT(onSetShortLongTimeout(void)));

                currentLayout->addWidget(shortLongTimeoutValue);
                currentLayout->addWidget(shortLongTimeoutSet);
                currentLayout->addWidget(shortLongTimeoutEdit);

                vLayout->addWidget(group);
            }

            // Double Tap Speed
            {
                QGroupBox* group = new QGroupBox("Double Tap Speed");
                QHBoxLayout* currentLayout = new QHBoxLayout;
                group->setLayout(currentLayout);

                doubleTapSpeedValue = new QLabel();

                doubleTapSpeedSet = new QPushButton("<-- Set");
                doubleTapSpeedEdit = new QLineEdit();
                doubleTapSpeedEdit->setValidator(new QIntValidator(0,0xFF));

                connect(doubleTapSpeedSet,SIGNAL(clicked(bool)),this,SLOT(onSetDoubleTapSpeed(void)));

                currentLayout->addWidget(doubleTapSpeedValue);
                currentLayout->addWidget(doubleTapSpeedSet);
                currentLayout->addWidget(doubleTapSpeedEdit);

                vLayout->addWidget(group);
            }

            hLayout->addLayout(vLayout);
        }

        // capabilities
        {
            capabilities = new QTextEdit;
            capabilities->setReadOnly(true);
            hLayout->addWidget(capabilities);
        }

        vLayout->addLayout(hLayout);
    }

    // Notifications
    {
        notifications = new QTextEdit;
        notifications->setReadOnly(true);
        notifications->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(notifications, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onNotificationContextMenu(const QPoint&)));
        vLayout->addWidget(notifications,1);
    }

    readInfo();
    readMode();

	bool bresult = devio::Subscriber<IFeature4600Crown::CrownEvent>::subscribe(feature4600);
    log->append(stringbuilder()
        << "Subscribing to 4600 events -> "
        << describe_err(bresult));

    addTab(page_state,"RatchetControlMode");
    addTab(log,"Log");
}

void f4600::readInfo()
{
    bool ok = feature4600->GetInfo(&_info);

    log->append( stringbuilder()
            << " GetInfo("
            << " button1=" << (int)_info.button1
            << ", button1LongPress=" << (int)_info.button1LongPress
            << ", ratchetControl=" << (int)_info.ratchetControl
            << ", rotationTimeout=" << (int)_info.rotationTimeout
            << ", shortLongTimeout=" << (int)_info.shortLongTimeout
            << ", doubleTapSpeed=" << (int)_info.doubleTapSpeed
            << ", proximity=" << (int)_info.proximity
            << ", touch=" << (int)_info.touch
            << ", tapGesture=" << (int)_info.tapGesture
            << ", doubleTapGesture=" << (int)_info.doubleTapGesture
            << ", slotsPerRevolution=" << (int)_info.slotsPerRevolution
            << ", ratchetsPerRevolution=" << (int)_info.ratchetsPerRevolution
            << ") -> " << describe_err(ok)
            );

    if (ok)
    {
        capabilities->setText("");
        capabilitiesOutput(QString("button1: %1").arg(_info.button1 ? "Yes" : "No"));
        capabilitiesOutput(QString("button1LongPress: %1").arg(_info.button1LongPress ? "Yes" : "No"));
        capabilitiesOutput(QString("proximity: %1").arg(_info.proximity ? "Yes" : "No"));
        capabilitiesOutput(QString("touch: %1").arg(_info.touch ? "Yes" : "No"));
        capabilitiesOutput(QString("tapGesture: %1").arg(_info.tapGesture ? "Yes" : "No"));
        capabilitiesOutput(QString("doubleTapGesture: %1").arg(_info.doubleTapGesture ? "Yes" : "No"));
        capabilitiesOutput(QString("slotsPerRevolution: %1").arg(_info.slotsPerRevolution));
        capabilitiesOutput(QString("ratchetsPerRevolution: %1").arg(_info.ratchetsPerRevolution));
    }
    else
    {
        capabilities->setText("fail");
        memset(&_info,0,sizeof(_info));
    }

    if (!_info.ratchetControl)
    {
        modeFreewheel->setEnabled(false);
        modeRatchet->setEnabled(false);
    }

    if (!_info.rotationTimeout)
    {
        rotationTimeoutSet->setEnabled(false);
        rotationTimeoutValue->setEnabled(false);
        rotationTimeoutEdit->setEnabled(false);
    }

    if (!_info.shortLongTimeout)
    {
        shortLongTimeoutSet->setEnabled(false);
        shortLongTimeoutValue->setEnabled(false);
        shortLongTimeoutEdit->setEnabled(false);
    }

    if (!_info.doubleTapSpeed)
    {
        doubleTapSpeedSet->setEnabled(false);;
        doubleTapSpeedValue->setEnabled(false);;
        doubleTapSpeedEdit->setEnabled(false);;
    }
}

void f4600::updateModeDisplay(struct IFeature4600Crown::CrownMode mode)
{
    page_state->blockSignals(true);
        
    switch (mode.reportingDiverted)
    {
    case IFeature4600Crown::Diverting::Hid:           reportingHid->setChecked(true); break;
    case IFeature4600Crown::Diverting::Diverted: reportingDiverted->setChecked(true); break;
    }

    switch (mode.ratchetMode)
    {
    case IFeature4600Crown::RatchetMode::Freewheel:  modeFreewheel->setChecked(true); break;
    case IFeature4600Crown::RatchetMode::Ratchet:      modeRatchet->setChecked(true); break;
    }

    rotationTimeoutValue->setText(QString("%1").arg(mode.rotationTimeout));
    shortLongTimeoutValue->setText(QString("%1").arg(mode.shortLongTimeout));
    doubleTapSpeedValue->setText(QString("%1").arg(mode.doubleTapSpeed));

    page_state->blockSignals(false);
}

void f4600::readMode()
{
    struct IFeature4600Crown::CrownMode mode;

    bool ok = feature4600->GetMode(&mode);

    log->append( stringbuilder()
            << " GetMode("
            << " reportingDiverted=" << (int)mode.reportingDiverted
            << ", ratchetMode=" << (int)mode.ratchetMode
            << ", rotationTimeout=" << (int)mode.rotationTimeout
            << ", shortLongTimeout=" << (int)mode.shortLongTimeout
            << ", doubleTapSpeed=" << (int)mode.doubleTapSpeed
            << ") -> " << describe_err(ok)
            );

    if (ok)
    {
        updateModeDisplay(mode);
    }
    else
    {
        notifyText("GetMode() failed", false);

        rotationTimeoutValue->setText("fail");
        shortLongTimeoutValue->setText("fail");
        doubleTapSpeedValue->setText("fail");
        reportingHid->setChecked(false);
        reportingDiverted->setChecked(false);
        modeFreewheel->setChecked(false);
        modeRatchet->setChecked(false);
    }
}

void f4600::setReportingDiverted(unsigned int reportingDiverted)
{
    struct IFeature4600Crown::CrownMode mode{};

    mode.reportingDiverted = (IFeature4600Crown::Diverting)reportingDiverted;
    bool ok = feature4600->SetMode(&mode);

    log->append( stringbuilder()
            << " SetMode("
            << " reportingDiverted=" << reportingDiverted
            << ") -> " << describe_err(ok)
            );
}

void f4600::setMode(unsigned int ratchetMode)
{
    struct IFeature4600Crown::CrownMode mode{};

    mode.ratchetMode = (IFeature4600Crown::RatchetMode)ratchetMode;
    bool ok = feature4600->SetMode(&mode);

    log->append( stringbuilder()
            << " SetMode("
            << " ratchetMode=" << ratchetMode
            << ") -> " << describe_err(ok)
            );

    if (!ok)
    {
        notifyText("SetMode() failed", false);
    }
}

void f4600::setRotationTimeout(unsigned int rotationTimeout)
{
    struct IFeature4600Crown::CrownMode mode{};

    mode.rotationTimeout = rotationTimeout;
    bool ok = feature4600->SetMode(&mode);

    log->append( stringbuilder()
            << " SetMode("
            << " rotationTimeout=" << rotationTimeout
            << ") -> " << describe_err(ok)
            );

    if (ok)
    {
        rotationTimeoutValue->setText(QString("%1").arg(mode.rotationTimeout));
    }
    else
    {
        rotationTimeoutValue->setText("fail");
    }
}

void f4600::setShortLongTimeout(unsigned int shortLongTimeout)
{
    struct IFeature4600Crown::CrownMode mode{};

    mode.shortLongTimeout = shortLongTimeout;
    bool ok = feature4600->SetMode(&mode);

    log->append( stringbuilder()
            << " SetMode("
            << " shortLongTimeout=" << shortLongTimeout
            << ") -> " << describe_err(ok)
            );

    if (ok)
    {
        shortLongTimeoutValue->setText(QString("%1").arg(mode.shortLongTimeout));
    }
    else
    {
        shortLongTimeoutValue->setText("fail");
    }
}

void f4600::setDoubleTapSpeed(unsigned int doubleTapSpeed)
{
    struct IFeature4600Crown::CrownMode mode{};

    mode.doubleTapSpeed = doubleTapSpeed;
    bool ok = feature4600->SetMode(&mode);

    log->append( stringbuilder()
            << " SetMode("
            << " doubleTapSpeed=" << doubleTapSpeed
            << ") -> " << describe_err(ok)
            );

    if (ok)
    {
        doubleTapSpeedValue->setText(QString("%1").arg(mode.doubleTapSpeed));
    }
    else
    {
        doubleTapSpeedValue->setText("fail");
    }
}

void f4600::onReportingGroupClicked(int id)
{
    setReportingDiverted(id);
}

void f4600::onModeGroupClicked(int id)
{
    setMode(id);
}

void f4600::onClickedSetRotationTimeout()
{
    setRotationTimeout(rotationTimeoutEdit->text().toUShort());
}

void f4600::onClickedShortLongTimeout()
{
    setShortLongTimeout(shortLongTimeoutEdit->text().toUShort());
}

void f4600::onClickedSetDoubleTapSpeed()
{
    setDoubleTapSpeed(doubleTapSpeedEdit->text().toUShort());
}

void f4600::notifyText(const QString &text, bool insertCRLF)
{
    // Jump to the end (to restore the cursor)
    QTextCursor tc = notifications->textCursor();
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    notifications->setTextCursor(tc);
    
    notifications->insertPlainText(text + QString("\n"));
    if (insertCRLF)
    {
        notifications->append("\n");
    }
    
    // Jump to the end (to scroll to the end)
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    notifications->setTextCursor(tc);
}

void f4600::capabilitiesOutput(const QString &text)
{
    // Jump to the end (to restore the cursor)
    QTextCursor tc = capabilities->textCursor();
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    capabilities->setTextCursor(tc);
    
    capabilities->insertPlainText(text + QString("\n"));
}

void f4600::onNotificationContextMenu(const QPoint &pos)
{
    QMenu popup(this);
    popup.addAction("Clear", this, SLOT(onClearNotifications()));
    popup.exec(notifications->mapToGlobal(pos));
}

void f4600::onClearNotifications(void)
{
    notifications->clear();
}

void f4600::settings_lost()
{
    notifyText("settings lost");
    log->append("settings lost");
    readMode();
}

static const char *stateStr(IFeature4600Crown::State value)
{
    switch (value)
    {
    case IFeature4600Crown::State::Inactive:  return "Inactive";
    case IFeature4600Crown::State::Start:     return "Start";
    case IFeature4600Crown::State::Active:    return "Active";
    case IFeature4600Crown::State::Stop:      return "Stop";
    }
    return "Invalid";
}

static const char *buttonStateStr(IFeature4600Crown::ButtonState value)
{
    switch (value)
    {
    case IFeature4600Crown::ButtonState::Inactive:    return "Inactive";
    case IFeature4600Crown::ButtonState::Start:       return "Start";
    case IFeature4600Crown::ButtonState::Short:       return "Short";
    case IFeature4600Crown::ButtonState::LongTrigger: return "LongTrigger";
    case IFeature4600Crown::ButtonState::Long:        return "Long";
    case IFeature4600Crown::ButtonState::Stop:        return "Stop";
    }
    return "Invalid";
}

static const char *gestureStr(IFeature4600Crown::Gesture value)
{
    switch (value)
    {
    case IFeature4600Crown::Gesture::None:      return "None";
    case IFeature4600Crown::Gesture::Tap:       return "Tap";
    case IFeature4600Crown::Gesture::DoubleTap: return "DoubleTap";
    }
    return "Invalid";
}

void f4600::onCrownEvent(struct IFeature4600Crown::CrownEventData data)
{
    auto s = stringbuilder();

    log->append( stringbuilder()
            << " CrownEvent("
            << " rotation=" << (int)data.rotation
            << ", slot=" << (int)data.rotationSlot
            << ", ratchet=" << (int)data.rotationRatchet
            << ", proximity=" << (int)data.proximity
            << ", touch=" << (int)data.touch
            << ", gesture=" << (int)data.gesture
            << ", button=" << (int)data.button
            << ")"
            );

    s << "Crown event:";
    s << " rotation=" << stateStr(data.rotation);

    if (data.rotationSlot)
    {
        s << ", slots=" << (int)data.rotationSlot;
    }

    if (data.rotationRatchet)
    {
        s << ", ratchets=" << (int)data.rotationRatchet;
    }

    if (data.proximity != IFeature4600Crown::State::Inactive)
    {
        s << ", proximity=" << stateStr(data.proximity);
    }

    if (data.touch != IFeature4600Crown::State::Inactive)
    {
        s << ", touch=" << stateStr(data.touch);
    }

    if (data.gesture != IFeature4600Crown::Gesture::None)
    {
        s << ", gesture=" << gestureStr(data.gesture);
    }

    if (data.button != IFeature4600Crown::ButtonState::Inactive)
    {
        s << ", button=" << buttonStateStr(data.button);
    }

///////////////////////////////////////////////////////////////////
    if (data.gesture == IFeature4600Crown::Gesture::Tap)
    {

        SendKeyEvent(29);    // zero    
    }
    else if (data.touch != IFeature4600Crown::State::Inactive)
    {
        if (data.rotationSlot != 0)
        {
            if (data.rotationSlot > 0)
            {
                SendKeyEvent(126);    // up  arrow

            }
            else if (data.rotationSlot < 0)
            {
                SendKeyEvent(125);    // down arrow

            }
        }
        else if (data.button != IFeature4600Crown::ButtonState::Inactive)
        {
            switch (data.button)
            {
            case IFeature4600Crown::ButtonState::Start:       SendKeyEvent(0x14);     break; // 3
            case IFeature4600Crown::ButtonState::Stop:        SendKeyEvent(0x15);    break; //4

            }
        }

        else
        {
            switch (data.touch)
            {
            case IFeature4600Crown::State::Inactive:  break;
            case IFeature4600Crown::State::Start:     SendKeyEvent(0x12);  break;// 1
            case IFeature4600Crown::State::Active:    break;// 
            case IFeature4600Crown::State::Stop:      SendKeyEvent(0x13);  break;// 2
            }
        }
              
    }

    


    notifyText(s,false);
}

void f4600::SendKeyEvent(int keycode)
{
    // no processing 
    return;

#if defined _WIN32
    // do nothign for now
    int key = 0;
    switch (keycode)
    {
    case 29:  key = 0x30;//0 
        break;
    case 0X12: key = 0x31;//1
        break;
    case 0X13: key = 0x32;//2
        break;
    case 0X14: key = 0x33;//3
        break;
    case 0X15: key = 0x34;//4
        break;

    case 125: key = VK_DOWN;//
        break;

    case 126: key = VK_UP;//
        break;
    }
    if (key != 0)
    {
        keybd_event(key, 0, 0, 0);
        Sleep(5);
        keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
    }

#elif defined __APPLE__
#if 1
    inject( keycode);
    
#endif 
#else
    // do nothing
#endif

}
void f4600::onModeChanged(unsigned int swid, struct IFeature4600Crown::CrownMode mode)
{
    // another app called SetMode() and we saw the response

    log->append( stringbuilder()
            << " external SetMode("
            << " reportingDiverted=" << (int)mode.reportingDiverted
            << ", ratchetMode=" << (int)mode.ratchetMode
            << ", rotationTimeout=" << (int)mode.rotationTimeout
            << ", shortLongTimeout=" << (int)mode.shortLongTimeout
            << ", doubleTapSpeed=" << (int)mode.doubleTapSpeed
            << ")"
            );

    updateModeDisplay(mode);
};

string f4600::timestamp(unsigned int int_timestamp)
{
    unsigned int sec = int_timestamp / 1000;
    unsigned int mils = int_timestamp - sec * 1000;

    sec = sec % 60;

    stringbuilder s;
    s << std::setfill('0') << std::setw(2) << sec << "." << std::setfill('0') << std::setw(3) << mils;
    return s;
}

unsigned int f4600::int_timestamp()
{
    using namespace std::chrono;

    auto duration = high_resolution_clock::now().time_since_epoch();
    auto milli = (unsigned int)duration_cast<std::chrono::milliseconds>(duration).count();

    return milli;
}
