#include "feature.h"
#include "f6100.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QString>

using devio::Delivery;

int modes[] = { 0,1,5,9,0x21,0x41,0xFFFF };
const int nummodes = sizeof(modes) / sizeof(int);

void f6100::rbutton_clicked(bool checked)
{
    (void)checked;

    for (int i=0; i < nummodes; i++)
    {
        if (radio_buttons[i]->isChecked())
        {
            int mode = modes[i];
            bool bresult = feature->SetRawReportState(mode);
            text->append(stringbuilder()
                << "Switched to mode 0x" << std::hex << mode << std::dec << " -> "
                << describe_err(bresult)
                );

            // read back setting to confirm
            settings_lost();

            return;
        }
    }

    text->append("No buttons are checked!");
}

f6100::f6100(shared_ptr<devio::IFeature6100TouchPadRawXY> feature6100, Feature* base) :
    QTabWidget(),
    feature(feature6100),
    baseFeature(base),
    mode(0)
{
     // textedit
    text = new QTextEdit;
    text->setPlainText(baseFeature->description);
    text->setReadOnly(true);
    text->setWordWrapMode(QTextOption::NoWrap);

    IFeature6100TouchPadRawXY::TouchPadInfo info;
    bool ok = feature6100->GetTouchPadInfo( &info);

    text->append( stringbuilder()
            << "GetTouchPadInfo("
            << " xSize=" << info.xSize
            << ", ySize=" << info.ySize
            << ", zDataRange=" << (unsigned int)info.zDataRange
            << ", areaDataRange=" << (unsigned int)info.areaDataRange
            << ", timeStampUnits=" << (unsigned int)info.timeStampUnits
            << ", maxFingerCount=" << (unsigned int)info.maxFingerCount
            << ", origin=" << (unsigned int)info.origin
            << ", penSupport=" << (unsigned int)info.penSupport
            << ", rawReportMappingVersion=" << (unsigned int)info.rawReportMmappingVersion
            << ", dpi=" << info.dpi
            << ", otherTouchpadInfo=" << (unsigned int)info.otherTouchpadInfo
            << ") -> " << describe_err(ok)
            );
 
    // controls
    controls = new QWidget;

    QGroupBox *groupBox = new QGroupBox(tr("Feature 6100 mode"));
    QVBoxLayout *vbox = new QVBoxLayout;
    for (int i=0; i < nummodes; i++)
    {
        auto radio_button = new QRadioButton(QString("Mode 0x%1").arg(modes[i],0,16));
        radio_button->setChecked(false);
        vbox->addWidget(radio_button);
        QObject::connect(radio_button,SIGNAL(clicked(bool)),this,SLOT(rbutton_clicked(bool)));
        radio_buttons.push_back(radio_button);
        if (i == nummodes - 1)
        {
            radio_button->hide();
        }
    }

    // read current setting
    settings_lost();

    //vbox->addStretch(1);
    groupBox->setLayout(vbox);
    QVBoxLayout *v = new QVBoxLayout;
    v->addWidget(groupBox);
    v->addStretch(1);
    controls->setLayout(v);

    // add tabs to tab widget
    addTab(controls,"controls");
    addTab(text,"text");

    // need to register this to use this type in a signal
    qRegisterMetaType<IFeature6100TouchPadRawXY::DualXYData>("IFeature6100TouchPadRawXY::DualXYData");

    bool connected = connect(this, SIGNAL(signalDualXYData(IFeature6100TouchPadRawXY::DualXYData)),
                             this, SLOT(slotDualXYData(IFeature6100TouchPadRawXY::DualXYData)),
                             Qt::QueuedConnection);
    if (!connected)
    {
        text->append("Error: signalDualXYData not connected to slotDualXYData!");
    }

    bool connected2 = connect(this, SIGNAL(signalModeChanged(unsigned int)),
                             this, SLOT(slotModeChanged(unsigned int)),
                             Qt::QueuedConnection);
    if (!connected2)
    {
        text->append("Error: signalModeChanged not connected to slotModeChanged!");
    }

    bool bresult = subscribe(feature6100,Delivery::Immediate);
    text->append(stringbuilder()
        << "Subscribing to 6100 reports -> "
        << describe_err(bresult)
        );
}


void f6100::slotDualXYData(const IFeature6100TouchPadRawXY::DualXYData data)
{
#if 0
    unsigned int contactType;
    unsigned int contactStatus;
    unsigned int x;
    unsigned int y;
    unsigned int z;
    unsigned int area;
    unsigned int fingerId;

    unsigned short timeStamp;
    struct finger finger1;
    struct finger finger2;
    bool endOfFrame;
    bool sp1;
    bool button;
    bool reserved;
    unsigned int numFingers;
#endif

    std::ostringstream s;
    s << data.timeStamp << " ";
    if (data.finger1.fingerId)
    {
        s << data.finger1.fingerId << ":(" << data.finger1.x << "," << data.finger1.y << ")";
        if (data.finger1.z)
        {
            s << std::hex << std::setfill('0') << std::setw(2) << data.finger1.z << std::dec;
        }
        if (data.finger1.area)
        {
            s << std::hex << ":" << std::setw(2) << data.finger1.area << " " << std::dec;
        }
    }
    if (data.finger2.fingerId)
    {
        s << data.finger2.fingerId << ":(" << data.finger2.x << "," << data.finger2.y << ")" ;
        if (data.finger2.z)
        {
            s << std::hex << std::setfill('0') << std::setw(2) << data.finger2.z << std::dec;
        }
        if (data.finger2.area)
        {
            s << std::hex << ":" << std::setw(2) << data.finger2.area << " " << std::dec;
        }
    }

    text->append(s.str().c_str());
}

void f6100::onDualXYData(const IFeature6100TouchPadRawXY::DualXYData data)
{
    // We are not in the gui thread.. send report to gui thread.
    emit signalDualXYData(data);
}

void f6100::slotModeChanged(unsigned int swid)
{
    // another sw changed the mode
    text->append(stringbuilder() << "mode changed by swid " << swid);

    settings_lost();
}

void f6100::onModeChanged(unsigned int swid)
{
    emit signalModeChanged(swid);
}

void f6100::settings_lost()
{
    devio::Byte oldmode = 0;
    bool bresult = feature->GetRawReportState(&oldmode);
    text->append(stringbuilder()
        << "GetRawReportState got mode 0x" << std::hex << (unsigned int)oldmode << std::dec << " -> "
        << describe_err(bresult)
        );

    for (unsigned int i=0; i < nummodes; i++)
    {
        if (radio_buttons.size() > i)
        {
            // last setting value is dynamic.. it adjusts to whatever nonstandard setting the device may have
            if (i == nummodes-1)
            {
                modes[i] = oldmode;
                radio_buttons[i]->setText(QString("Mode 0x%1  (nonstandard)").arg(modes[i],0,16));
                radio_buttons[i]->show();
            }

            if (oldmode == modes[i])
            {
                radio_buttons[i]->blockSignals(true);
                radio_buttons[i]->setChecked(true);
                radio_buttons[i]->blockSignals(false);
                break;
            }
        }
    }
}

