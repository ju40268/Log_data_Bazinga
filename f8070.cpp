#include "feature.h"
#include "f8070.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include "f8070SyncDialog.h"


using devio::Delivery;

#ifndef _WIN32

#ifndef _countof
// exists in VS 2012, but not in Xcode 4.6.3
// from: http://blogs.msdn.com/b/the1/archive/2004/05/07/128242.aspx
template <typename T, size_t N>
char ( &_ArraySizeHelper( T (&array)[N] ))[N];
#define _countof( array ) (sizeof( _ArraySizeHelper( array ) ))
#endif // _countof

#endif // _WIN32

const f8070::EffectUsageHint f8070::_usageHints[] = {
    { IFeature8070ColorLEDEffects::EF_OFF },
    { IFeature8070ColorLEDEffects::EF_FIXED,            { "(Red)", "(Green)", "(Blue)", "Ramp (0-2)" } },
    { IFeature8070ColorLEDEffects::EF_PULSE,            { "(Red)", "(Green)", "(Blue)" , "Delay (ms)"} },
    { IFeature8070ColorLEDEffects::EF_CYCLE,            { "", "", "", "", "", "Speed (ms) MSB", "Speed (ms) LSB", "Intensity (1-100)"} },
    { IFeature8070ColorLEDEffects::EF_COLOR_WAVE,       { "", "", "", "", "", "", "Speed (ms) LSB", "Direction (1-3)", "Intensity (1-100)", "Speed (ms) MSB" } },
    { IFeature8070ColorLEDEffects::EF_STARLIGHT,        { "Red (Sky)", "Green (Sky)", "Blue (Sky)", "Red (Star)", "Green (Star)", "Blue (Star)", "Delay (ms)" } },
    { IFeature8070ColorLEDEffects::EF_LIGHT_ON_PRESS,   { "(Red)", "(Green)", "(Blue)", "Delay (ms)" } },
    { IFeature8070ColorLEDEffects::EF_AUDIO },
    { IFeature8070ColorLEDEffects::EF_BOOT_UP },
    { IFeature8070ColorLEDEffects::EF_DEMO },
    { IFeature8070ColorLEDEffects::EF_PULSE_WAVEFORM, { "(Red)", "(Green)", "(Blue)", "Speed (ms) MSB", "Speed (ms) LSB", "Waveform (1-5)", "Intensity (1-100)" } },
};

const f8070::NonVolatileSetting f8070::_nonVolatileSettings[] = {
    { IFeature8070ColorLEDEffects::BOOTUP_EFFECT,       "BOOT_UP_EFFECT"            },
    { IFeature8070ColorLEDEffects::DEMO,                "DEMO_MODE"                 },
};

const f8070::ExtendedCapabilities f8070::_extendedCapabilities[] = {
    { IFeature8070ColorLEDEffects::EXT_GETZONEEFFECT,           "GET_ZONE_EFFECT"       },
    { IFeature8070ColorLEDEffects::EXT_NO_GETEFFECTSETTINGS,    "GET_EFFECT_SETTINGS"   },
    { IFeature8070ColorLEDEffects::EXT_SETLEDBININFO,           "SET_LED_BIN_INFO"      },
    { IFeature8070ColorLEDEffects::EXT_MONOCHROME,              "MONOCHROME"            },
};


//*************************************************************************
//
// f8070::f8070
//
//*************************************************************************

f8070::f8070(shared_ptr<devio::IFeature8070ColorLEDEffects> f, Feature* base) 
{
    feature8070 = f;
    m_setEffectOption = IFeature8070ColorLEDEffects::SET_EFFECT_RAM;
    createUI();
}


//*************************************************************************
//
// f8070::createUI
//
//*************************************************************************

void f8070::createUI(void)
{
    QVBoxLayout* vbox = new QVBoxLayout;

    // Create the zones as a vertical list of group boxes
    int zoneCount = 0;
    _nvCapabilities = IFeature8070ColorLEDEffects::NV_INVALID;
    _extCapabilities = IFeature8070ColorLEDEffects::EXT_NONE;
    if (!feature8070->GetInfo(zoneCount, _nvCapabilities, _extCapabilities))
    {
        vbox->addWidget(new QLabel("Failed to get zone count"));
    }

    {
        QLabel* nvCapsLabel = new QLabel;
        QString nvCapsStr;

        for (int i = 0; i < _countof(_nonVolatileSettings); i++)
        {
            if (_nvCapabilities & _nonVolatileSettings[i].capability)
            {
                nvCapsStr += QString(_nonVolatileSettings[i].name + " ");
            }
        }
        nvCapsLabel->setText(QString("info.nvCaps (0x%1): %2").arg(_nvCapabilities, 0, 16).arg(nvCapsStr));
        vbox->addWidget(nvCapsLabel);
    }

    {
        QLabel* extCapsLabel = new QLabel;
        QString exrCapsStr;
        for (int i = 0; i < _countof(_extendedCapabilities); i++)
        {
            if (_extCapabilities & _extendedCapabilities[i].capability)
            {
                exrCapsStr += QString(_extendedCapabilities[i].name + " ");
            }
        }
        extCapsLabel->setText(QString("info.extCaps (0x%1): %2").arg(_extCapabilities, 0, 16).arg(exrCapsStr));
        vbox->addWidget(extCapsLabel);
    }

    vbox->addSpacerItem(new QSpacerItem(0, 10));

    QLabel* zonesLabel = new QLabel("Zone:");
    _zoneList   = new QComboBox;
    QLabel* effectsLabel = new QLabel("Effect:");
    _effectsList = new QComboBox;
    _effectInfo = new QLabel;
    _effectInfo->setWordWrap(true);

    // Add getColor preview (if feature is > v1) 
    QGroupBox* previewGroup = new QGroupBox("Current Device Color");
    if (featureVersion() >= 1)
    {
        previewGroup->setLayout(new QVBoxLayout());
        QPushButton* getCurrentColor = new QPushButton("Get Current Color");
        QCheckBox* enablePolling = new QCheckBox("Enable Polling (50ms)");
        _enableInverseGammaCorrection = new QCheckBox("Inverse Gamma Correction (from device to UI)");

        QWidget* colorPreview = new QWidget;
        colorPreview->setLayout(new QHBoxLayout);
        colorPreview->layout()->setContentsMargins(0, 0, 0, 0);
        QPixmap p(16, 16);
        p.fill(Qt::transparent);
        _rgbPreview = new QLabel;
        _colorPreview = new QLabel;
        _colorPreview->setPixmap(p);
        colorPreview->layout()->addWidget(_colorPreview);
        colorPreview->layout()->addWidget(_rgbPreview);
        ((QHBoxLayout*)colorPreview->layout())->addStretch();

        previewGroup->layout()->addWidget(getCurrentColor);
        previewGroup->layout()->addWidget(enablePolling);
        previewGroup->layout()->addWidget(_enableInverseGammaCorrection);
        previewGroup->layout()->addWidget(colorPreview);
        _colorTimer = new QTimer(this);
        _colorTimer->setInterval(50);
        _colorTimer->setSingleShot(false);

        connect(getCurrentColor, SIGNAL(clicked()), this, SLOT(onGetCurrentColor()));
        connect(_colorTimer, SIGNAL(timeout()), this, SLOT(onGetCurrentColor()));
        connect(enablePolling, SIGNAL(toggled(bool)), this, SLOT(onColorTimerToggled(bool)));
    }

    vbox->addWidget(zonesLabel);
    vbox->addWidget(_zoneList);
    vbox->addSpacing(20);
    vbox->addWidget(effectsLabel);
    vbox->addWidget(_effectsList);
    vbox->addWidget(_effectInfo);

    connect(_zoneList, SIGNAL(currentIndexChanged(int)), this, SLOT(onZoneChanged(int)));
    connect(_effectsList, SIGNAL(currentIndexChanged(int)), this, SLOT(onZoneEffectChanged(int)));

    // Let's put each zone into a group box
    // Add a selector to select the effect for that zone
    // Create an 'editor' for each effect
    // And then show/hide that editor when it is selected
    for (int zoneIndex = 0; zoneIndex < zoneCount; zoneIndex++)
    {
        IFeature8070ColorLEDEffects::ZoneInfo zoneInfo;
        memset(&zoneInfo, 0, sizeof(zoneInfo));
        feature8070->GetZoneInfo(zoneIndex, zoneInfo);
        _zoneList->addItem(toString(zoneInfo.location), zoneInfo.numEffects);
    }

    {
        vbox->addSpacing(20);
        QFrame* separator = new QFrame();
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        vbox->addWidget(separator);
        vbox->addSpacing(20);
    }

    // Add parameters
    QLabel* paramsLabel = new QLabel("Parameters:");
    vbox->addWidget(paramsLabel);
    vbox->addSpacing(3);

    // Add a grid layout with 10 params
    _paramsGrid = new QGridLayout;
    for (int i = 0; i < 10; i++)
    {
        QLabel* paramLabel = new QLabel(QString("%1").arg(i+1));
        paramLabel->setAlignment(Qt::AlignCenter);
        _paramsGrid->addWidget(paramLabel, 0, i);

        _params[i] = new QLineEdit;
        _paramsGrid->addWidget(_params[i], 1, i);

        // Add hints (we'll change them later)
         QLabel* paramHint = new QLabel;
         paramHint->setAlignment(Qt::AlignCenter);
        _paramsGrid->addWidget(paramHint, 2, i);
    }

    vbox->addLayout(_paramsGrid);


    {
        vbox->addSpacing(20);

        QCheckBox* enableSWControl = new QCheckBox("Enable SW Control");
        bool swControlEnabled = false;
        bool syncEnabled = false;
        if (feature8070->GetSWControl(swControlEnabled, syncEnabled))
        {
            enableSWControl->setChecked(swControlEnabled);
        }
        connect(enableSWControl, SIGNAL(toggled(bool)), this, SLOT(onSWControlToggled(bool)));
        vbox->addWidget(enableSWControl);

        _enableGammaCorrection = new QCheckBox("Gamma Correction (from UI to device)");
        vbox->addWidget(_enableGammaCorrection);

        {
            QGroupBox* setEffectGroup = new QGroupBox("Set Effect");
            setEffectGroup->setLayout(new QVBoxLayout);
            QPushButton* apply = new QPushButton("Set Effect");
            QButtonGroup* setEffectOptions = new QButtonGroup();
            
            setEffectOptions->addButton(new QRadioButton("RAM"), IFeature8070ColorLEDEffects::SET_EFFECT_RAM);
            setEffectOptions->addButton(new QRadioButton("RAM+EEPROM"), IFeature8070ColorLEDEffects::SET_EFFECT_RAM_EEPROM);
            setEffectOptions->addButton(new QRadioButton("EEPROM"), IFeature8070ColorLEDEffects::SET_EFFECT_EEPROM);
            setEffectOptions->button((int)IFeature8070ColorLEDEffects::SET_EFFECT_RAM)->setChecked(true);

            connect(apply, SIGNAL(clicked()), this, SLOT(onApplyEffect()));
            connect(setEffectOptions, SIGNAL(buttonClicked(int)), this, SLOT(onSetEffectOptionChanged(int)));

            apply->setFixedWidth(200);
            
            setEffectGroup->layout()->addWidget(setEffectOptions->button((int)IFeature8070ColorLEDEffects::SET_EFFECT_RAM));
            setEffectGroup->layout()->addWidget(setEffectOptions->button((int)IFeature8070ColorLEDEffects::SET_EFFECT_RAM_EEPROM));
            setEffectGroup->layout()->addWidget(setEffectOptions->button((int)IFeature8070ColorLEDEffects::SET_EFFECT_EEPROM));
            setEffectGroup->layout()->addWidget(apply);

            if (featureVersion() < 3)
            {
                setEffectOptions->button((int)IFeature8070ColorLEDEffects::SET_EFFECT_RAM_EEPROM)->setDisabled(true);
                setEffectOptions->button((int)IFeature8070ColorLEDEffects::SET_EFFECT_EEPROM)->setDisabled(true);
            }

            vbox->addWidget(setEffectGroup);        
        }

        // Add a button to get the current settings
        QPushButton* getCurrentEffectSettings = new QPushButton("Get Effect Settings");
        getCurrentEffectSettings->setFixedWidth(200);
        connect(getCurrentEffectSettings, SIGNAL(clicked()), this, SLOT(onGetCurrentEffectSettings()));
        vbox->addWidget(getCurrentEffectSettings);
        if (_extCapabilities & IFeature8070ColorLEDEffects::EXT_NO_GETEFFECTSETTINGS)
        {
            getCurrentEffectSettings->setDisabled(true);
        }
    }

    // Notes
    vbox->addWidget(new QLabel("NOTE: Colors have values from 0-255. For hex values, prefix with '0x'"));
    vbox->addSpacing(10);

    vbox->addSpacing(5);
    createNonVolatileUI(vbox);
    vbox->addSpacing(5);

    if (featureVersion() >= 1)
    {
        vbox->addSpacing(10);
        vbox->addWidget(previewGroup);    

        // Effect Sync Tester
        vbox->addSpacing(10);
        QPushButton* syncDialog = new QPushButton("Effect Sync Tester");
        connect(syncDialog, SIGNAL(clicked()), this, SLOT(onSyncDialog()));
        vbox->addWidget(syncDialog);
    }

    // Saving to EEPROM
    if (featureVersion() >= 3)
    {
        QPushButton* getZoneEffect = new QPushButton("getZoneEffect");

        connect(getZoneEffect, SIGNAL(clicked()), this, SLOT(onGetZoneEffect()));
        vbox->addWidget(getZoneEffect);
        vbox->addSpacing(20);

        if (0 == (_extCapabilities & IFeature8070ColorLEDEffects::EXT_GETZONEEFFECT))
        {
            getZoneEffect->setDisabled(true);
        }
    }

    // Notifications
    notifications = new QTextEdit;
    notifications->setReadOnly(true);
    notifications->setWordWrapMode(QTextOption::NoWrap);
    notifications->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(notifications, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenu(const QPoint&)));
    vbox->addWidget(notifications);

    setLayout(vbox);

    // Select the first zone/effect
    _zoneList->setCurrentIndex(0);
    _effectsList->setCurrentIndex(0);

    // Add some info
    notifications->append(stringbuilder() << "Feature Version:" << featureVersion());
}


//*************************************************************************
//
// f8070::toString
//
//*************************************************************************

QString f8070::toString(const IFeature8070ColorLEDEffects::ZoneLocation& loc)
{
    switch(loc)
    {
    case IFeature8070ColorLEDEffects::ZONE_PRIMARY:
        return "Primary";
    case IFeature8070ColorLEDEffects::ZONE_LOGO:
        return "Logo";
    case IFeature8070ColorLEDEffects::ZONE_LEFT_SIDE:
        return "Left Side";
    case IFeature8070ColorLEDEffects::ZONE_RIGHT_SIDE:
        return "Right Side";
    case IFeature8070ColorLEDEffects::ZONE_COMBINED:
        return "Combined";
    case IFeature8070ColorLEDEffects::ZONE_ZONAL_ONE:
        return "Zonal 1";
    case IFeature8070ColorLEDEffects::ZONE_ZONAL_TWO:
        return "Zonal 2";
    case IFeature8070ColorLEDEffects::ZONE_ZONAL_THREE:
        return "Zonal 3";
    case IFeature8070ColorLEDEffects::ZONE_ZONAL_FOUR:
        return "Zonal 4";
    case IFeature8070ColorLEDEffects::ZONE_ZONAL_FIVE:
        return "Zonal 5";
    case IFeature8070ColorLEDEffects::ZONE_ZONAL_SIX:
        return "Zonal 6";
    default:
        return "Unknown";
    }
}


//*************************************************************************
//
// f8070::toString
//
//*************************************************************************

QString f8070::toString(const IFeature8070ColorLEDEffects::EffectId& id)
{
    switch(id)
    {
    case IFeature8070ColorLEDEffects::EF_OFF:
        return "Off";
    case IFeature8070ColorLEDEffects::EF_FIXED:
        return "Fixed Color";
    case IFeature8070ColorLEDEffects::EF_PULSE:
        return "Breathing/Pulsing";
    case IFeature8070ColorLEDEffects::EF_CYCLE:
        return "Cycling";
    case IFeature8070ColorLEDEffects::EF_COLOR_WAVE:
        return "Color Wave";
    case IFeature8070ColorLEDEffects::EF_STARLIGHT:
        return "Starlight";
    case IFeature8070ColorLEDEffects::EF_LIGHT_ON_PRESS:
        return "Light on Press";
    case IFeature8070ColorLEDEffects::EF_AUDIO:
        return "Audio";
    case IFeature8070ColorLEDEffects::EF_BOOT_UP:
        return "Boot Up";
    case IFeature8070ColorLEDEffects::EF_DEMO:
        return "Demo";
    case IFeature8070ColorLEDEffects::EF_PULSE_WAVEFORM:
        return "Pulse (Waveform)";
    default:
        return QString("Unknown (effect_id=%d)").arg(0, id);
    }
}


//*************************************************************************
//
// f8070::toString
//
//*************************************************************************

QString f8070::toString(const IFeature8070ColorLEDEffects::EffectInfo& effectInfo)
{
    quint16 effectCapabilities = effectInfo.capabilities;
    quint16 effectId = effectInfo.id;

    QString result = QString("Capabilities (0x%1): ").arg((int)effectCapabilities, 0, 16);

    if ((effectCapabilities > 0) && !(effectCapabilities & 0x01))
    {
        result += QString("ERROR: Bit 1 not set");
        return result;
    }

    switch (effectId)
    {
        case IFeature8070ColorLEDEffects::EF_OFF:
            return QString();
        case IFeature8070ColorLEDEffects::EF_FIXED:
        {
            if (IFeature8070ColorLEDEffects::FIXED_RAMP_UP_DOWN & effectCapabilities)
            {
                result += "Ramp";
            }
        }
            break;
        case IFeature8070ColorLEDEffects::EF_CYCLE:
        {
            if (IFeature8070ColorLEDEffects::CYCLE_SPEED & effectCapabilities)
            {
                result += "8BitPeriod ";
            }
            if (IFeature8070ColorLEDEffects::CYCLE_INTENSITY & effectCapabilities)
            {
                result += "Intensity ";
            }
            if (IFeature8070ColorLEDEffects::CYCLE_SYN_EFFECT_CAPABILITY & effectCapabilities)
            {
                result += "Sync ";
            }
            if (IFeature8070ColorLEDEffects::CYCLE_PERIOD & effectCapabilities)
            {
                result += "16BitPeriod ";
            }
            result += QString("\nMinPeriod=%1ms").arg(effectInfo.period);
        }
            break;
        case IFeature8070ColorLEDEffects::EF_COLOR_WAVE:
        {
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_START_COLOR & effectCapabilities)
            {
                result += "StartColor ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_STOP_COLOR & effectCapabilities)
            {
                result += "StopColor ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_SPEED & effectCapabilities)
            {
                result += "8BitPeriod ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_INTENSITY & effectCapabilities)
            {
                result += "Intensity  ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_HORIZONTAL & effectCapabilities)
            {
                result += "Horizontal ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_VERTICAL & effectCapabilities)
            {
                result += "Vertical  ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_CENTER_OUT & effectCapabilities)
            {
                result += "CenterOut ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_INWARD & effectCapabilities)
            {
                result += "Inward ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_OUTWARD & effectCapabilities)
            {
                result += "Outward ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_REVERSE_HORIZONTAL & effectCapabilities)
            {
                result += "ReverseHorizontal ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_REVERSE_VERTICAL & effectCapabilities)
            {
                result += "ReverseVertical ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_CENTER_IN & effectCapabilities)
            {
                result += "CenterIn ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_SYNC_EFFECT_CAPABLE & effectCapabilities)
            {
                result += "Sync ";
            }
            if (IFeature8070ColorLEDEffects::COLOR_WAVE_PERIOD & effectCapabilities)
            {
                result += "16BitPeriod ";
            }
            result += QString("\nMinPeriod=%1ms").arg(effectInfo.period);
        }
            break;
        case IFeature8070ColorLEDEffects::EF_PULSE_WAVEFORM:
        {
            if (IFeature8070ColorLEDEffects::PULSE_WAVEFORM_SPEED & effectCapabilities)
            {
                result += "8BitPeriod ";
            }
            if (IFeature8070ColorLEDEffects::PULSE_WAVEFORM_INTENSITY & effectCapabilities)
            {
                result += "Intensity ";
            }
            if (IFeature8070ColorLEDEffects::PULSE_WAVEFORM_SINE & effectCapabilities)
            {
                result += "Sine ";
            }
            if (IFeature8070ColorLEDEffects::PULSE_WAVEFORM_SQUARE & effectCapabilities)
            {
                result += "Square  ";
            }
            if (IFeature8070ColorLEDEffects::PULSE_WAVEFORM_TRIANGLE & effectCapabilities)
            {
                result += "Triangle ";
            }
            if (IFeature8070ColorLEDEffects::PULSE_WAVEFORM_SAWTOOTH & effectCapabilities)
            {
                result += "Sawtooth  ";
            }
            if (IFeature8070ColorLEDEffects::PULSE_WAVEFORM_SHARKFIN & effectCapabilities)
            {
                result += "Sharkfin ";
            }
            if (IFeature8070ColorLEDEffects::PULSE_WAVEFORM_EXPONENTIAL & effectCapabilities)
            {
                result += "Exponential ";
            }
            if (IFeature8070ColorLEDEffects::PULSE_WAVE_SYNC_EFFECT_CAPABLE & effectCapabilities)
            {
                result += "Sync ";
            }
            if (IFeature8070ColorLEDEffects::PULSE_WAVEFORM_PERIOD & effectCapabilities)
            {
                result += "16BitPeriod ";
            }
            result += QString("\nMinPeriod=%1ms").arg(effectInfo.period);
        }
            break;
        default:
        case IFeature8070ColorLEDEffects::EF_STARLIGHT:
        case IFeature8070ColorLEDEffects::EF_LIGHT_ON_PRESS:
        case IFeature8070ColorLEDEffects::EF_AUDIO:
        case IFeature8070ColorLEDEffects::EF_BOOT_UP:
        case IFeature8070ColorLEDEffects::EF_DEMO:
        case IFeature8070ColorLEDEffects::EF_PULSE:
            break;
    }

    return result;
}



//*************************************************************************
//
// f8070::onZoneChanged
//
//*************************************************************************

void f8070::onZoneChanged(int zoneIndex)
{
    _effectsList->blockSignals(true);
    _effectsList->clear();

    int zoneEffects = _zoneList->itemData(zoneIndex).toInt();
    for (int effectIndex = 0; effectIndex < zoneEffects; effectIndex++)
    {
        IFeature8070ColorLEDEffects::EffectInfo effectInfo;
        feature8070->GetZoneEffectInfo(zoneIndex, effectIndex, effectInfo);
        _effectsList->addItem(toString(effectInfo.id), effectInfo.id);
    }

    // Select the first one
    _effectsList->setCurrentIndex(0);

    _effectsList->blockSignals(false);
}


//*************************************************************************
//
// f8070::onZoneEffectChanged
//
//*************************************************************************

void f8070::onZoneEffectChanged(int index)
{
    if (index >= sizeof(_usageHints))
    {
        return;
    }

    const EffectUsageHint* usageHint = NULL;
    IFeature8070ColorLEDEffects::EffectId id = (IFeature8070ColorLEDEffects::EffectId)_effectsList->itemData(index).toInt();
    int usageHintsCount = sizeof(_usageHints) / sizeof(_usageHints[0]);
    for (int i = 0; i < usageHintsCount; i++)
    {
        if (_usageHints[i].id == id)
        {
            usageHint = &_usageHints[i];
            break;
        }
    }
    if (NULL == usageHint)
    {
        return;
    }

    // Change the param hints
    for (int i = 0; i < 10; i++)
    {
        QLayoutItem* item = _paramsGrid->itemAtPosition(2, i);
        if (NULL == item)
        {
            break;
        }

        QLabel *label = qobject_cast<QLabel *>(item->widget());
        if (NULL == label)
        {
            break;
        }
        label->setText(usageHint->params[i]);

        if (usageHint->params[i].isEmpty())
        {
            _params[i]->setText("");
        }
    }

    // Get effect caps
    int zoneIndex = _zoneList->currentIndex();
    IFeature8070ColorLEDEffects::EffectInfo effectInfo;
    feature8070->GetZoneEffectInfo(zoneIndex, index, effectInfo);
    _effectInfo->setText(toString(effectInfo));

}


//*************************************************************************
//
// f8070::onApplyEffect
//
//*************************************************************************

void f8070::onApplyEffect(void)
{
    int zoneIndex = _zoneList->currentIndex();
    int zoneEffectIndex = _effectsList->currentIndex();

    // Collect the params
    vector<Byte> params;
    for (int i = 0; i < 10; i++)
    {
        params.push_back(_params[i]->displayText().toUShort(NULL, 0));
    }

    if (_enableGammaCorrection->isChecked())
    {
        // Apply a gamma correction before sending it off
        switch (zoneEffectIndex)
        {
            case IFeature8070ColorLEDEffects::EF_FIXED:
            case IFeature8070ColorLEDEffects::EF_CYCLE:
            case IFeature8070ColorLEDEffects::EF_PULSE_WAVEFORM:
            {
                QColor before(params[0], params[1], params[2]);
                QColor after = gamma(before);
                params[0] = after.red();
                params[1] = after.green();
                params[2] = after.blue();
            }
            default:
                break;
        }
    }

    // Param 11 is now persistence
    params.push_back((quint8)m_setEffectOption);

    feature8070->SetZoneEffectByIndex(zoneIndex, zoneEffectIndex, params);
}


//*************************************************************************
//
// f8070::gamma
//
//*************************************************************************

QColor f8070::gamma(const QColor& color)
{
    QColor result = color;
    for (int i = 0; i < 3; i++)
    {
        float ratio;
        float multiplier;
        switch (i)
        {
            case 0: ratio = result.red(); break;
            case 1: ratio = result.green(); break;
            case 2: ratio = result.blue(); break;
            default:
                break;
        }
        ratio /= 255.0f;
        //sRGB equation
        if (ratio < .04045)
        {
            multiplier = ratio / 12.92;
        }
        else
        {
            multiplier = powf(((ratio + .055f) / 1.055f), 2.4f);
        }
        int newVal = 255 * multiplier;
        switch (i)
        {
            case 0: result.setRed(newVal); break;
            case 1: result.setGreen(newVal); break;
            case 2: result.setBlue(newVal); break;
            default:
                break;
        }
    }
    return result;
}


//*************************************************************************
//
// f8070::inverseGamma
//
//*************************************************************************

QColor f8070::inverseGamma(const QColor& color)
{
    QColor result = color;
    for (int i = 0; i < 3; i++)
    {
        float ratio;
        float multiplier;
        switch (i)
        {
            case 0: ratio = result.red(); break;
            case 1: ratio = result.green(); break;
            case 2: ratio = result.blue(); break;
            default:
                break;
        }
        ratio /= 255.0f;
        //sRGB equation
        if (ratio <= .0031308f)
        {
            multiplier = ratio * 12.92;
        }
        else
        {
            multiplier = (1.055) * powf(ratio, 1 / 2.4f) - .055f;
        }
        int newVal = 255 * multiplier;
        switch (i)
        {
            case 0: result.setRed(newVal); break;
            case 1: result.setGreen(newVal); break;
            case 2: result.setBlue(newVal); break;
            default:
                break;
        }
    }
    return result;

}


//*************************************************************************
//
// f8070::dumpZoneEffect
//
//*************************************************************************

void f8070::dumpZoneEffect(int zoneIndex, int zoneEffectIndex, vector<Byte>& effectParams)
{
    notifications->append(QString("Zone Index: %1, Effect Index: %2").arg(zoneEffectIndex).arg(zoneEffectIndex));
    for (size_t i = 0; i < effectParams.size(); i++)
    {
        notifications->append(QString("   [%1]: 0x%2").arg(i).arg((unsigned char)effectParams[i], 0, 16));
    }
}


void f8070::createNonVolatileUI(QLayout* mainLayout)
{
    QGroupBox* nvGroup = new QGroupBox("Non-Volatile Settings");

    // Add a grid layout
    QGridLayout* nvGrid = new QGridLayout;
    // Labels
    nvGrid->addWidget(new QLabel("Setting"), 0, 0);
    nvGrid->addWidget(new QLabel("Value"), 0, 1);
    nvGrid->addWidget(new QLabel("Param 1"), 0, 2);
    nvGrid->addWidget(new QLabel("Param 2"), 0, 3);

    // Edits/Buttons
    _nvSettingsList = new QComboBox;
    for (int i = 0; i < _countof(_nonVolatileSettings); i++)
    {
        if (_nvCapabilities & _nonVolatileSettings[i].capability)
        {
            _nvSettingsList->addItem(_nonVolatileSettings[i].name, _nonVolatileSettings[i].capability);        
        }
    }
    _nvSettingsList->setCurrentIndex(0);

    _nvValue = new QLineEdit;
    _nvValue->setValidator(new QIntValidator(0, 255));
    _nvParam1 = new QLineEdit;
    _nvParam1->setValidator(new QIntValidator(0, 255));
    _nvParam2 = new QLineEdit;
    _nvParam2->setValidator(new QIntValidator(0, 255));

    nvGrid->addWidget(_nvSettingsList, 1, 0);
    nvGrid->addWidget(_nvValue, 1, 1);
    nvGrid->addWidget(_nvParam1, 1, 2);
    nvGrid->addWidget(_nvParam2, 1, 3);

    QPushButton* getNvConfig = new QPushButton("Get");
    connect(getNvConfig, SIGNAL(clicked()), this, SLOT(getSelectedNvConfig()));
    QPushButton* setNvConfig = new QPushButton("Set");
    connect(setNvConfig, SIGNAL(clicked()), this, SLOT(setSelectedNvConfig()));
    nvGrid->addWidget(getNvConfig, 1, 4);
    nvGrid->addWidget(setNvConfig, 1, 5);

    nvGroup->setLayout(nvGrid);
    mainLayout->addWidget(nvGroup);
}


//*************************************************************************
//
// f8070::onGetCurrentColor
//
//*************************************************************************

void f8070::onGetCurrentColor(void)
{
    int zoneIndex = _zoneList->currentIndex();
    int r, g, b;
    if (feature8070->GetCurrentColor(zoneIndex, r, g, b))
    {
        QColor color(r, g, b);
        if (_enableInverseGammaCorrection->isChecked())
        {
            color = inverseGamma(color);
        }

        QPixmap p(16, 16);
        p.fill(color);
        _colorPreview->setPixmap(p);

        _rgbPreview->setText(QString("R:%1(0x%2) G:%3(0x%4) B:%5(0x%6)")
            .arg(color.red()).arg(color.red(), 0, 16).arg(color.green()).arg(color.green(), 0, 16).arg(color.blue()).arg(color.blue(), 0, 16));
    }
    else
    {
        notifications->append("GetCurrentColor failed (or not supported)");
    }
}


//*************************************************************************
//
// f8070::onGetCurrentColor
//
//*************************************************************************

void f8070::onColorTimerToggled(bool checked)
{
    checked ? _colorTimer->start() : _colorTimer->stop();
}


//*************************************************************************
//
// f8070::onSWControlToggled
//
//*************************************************************************

void f8070::onSWControlToggled(bool checked)
{
    feature8070->SetSWControl(checked, false);
}


//*************************************************************************
//
// f8070::featureVersion
//
//*************************************************************************

int f8070::featureVersion(void)
{
    // Cast to Feature
    shared_ptr<devio::Feature> feature = std::dynamic_pointer_cast<devio::Feature>(feature8070);
    if (!feature)
    {
        return 0;
    }

    return feature->version();
}


//*************************************************************************
//
// f8070::onGetZoneEffect
//
//*************************************************************************

void f8070::onGetZoneEffect(void)
{
    int zoneEffectIndex = -1;
    vector<Byte> effectParams;

    // RAM
    effectParams.clear();
    notifications->append("GetZoneEffect (RAM)");
    if (feature8070->GetZoneEffect(_zoneList->currentIndex(), zoneEffectIndex, effectParams, false))
    {
        dumpZoneEffect(_zoneList->currentIndex(), zoneEffectIndex, effectParams);
    }
    else
    {
        notifications->append("  FAILED");
    }

    // EEPROM
    zoneEffectIndex = -1;
    effectParams.clear();
    notifications->append("GetZoneEffect (EEPROM)");
    if (feature8070->GetZoneEffect(_zoneList->currentIndex(), zoneEffectIndex, effectParams, true))
    {
        dumpZoneEffect(_zoneList->currentIndex(), zoneEffectIndex, effectParams);
    }
    else
    {
        notifications->append("  FAILED");
    }
}


//*************************************************************************
//
// f8070::onSetEffectOptionChanged
//
//*************************************************************************

void f8070::onSetEffectOptionChanged(int option)
{
    m_setEffectOption = (IFeature8070ColorLEDEffects::SetEffectOption)option;
}


//*************************************************************************
//
// f8070::onContextMenu
//
//*************************************************************************

void f8070::onContextMenu(const QPoint& pt)
{
    QMenu popup(this);
    popup.addAction("Clear", notifications, SLOT(clear()));
    popup.exec(notifications->mapToGlobal(pt));
}


//*************************************************************************
//
// f8070::onSyncDialog
//
//*************************************************************************

void f8070::onSyncDialog(void)
{
    static f8070SyncDialog* syncDialog  = NULL;
    if (NULL == syncDialog)
    {
        syncDialog = new f8070SyncDialog();
    }

    syncDialog->show();
}


//*************************************************************************
//
// f8070::getSelectedNvConfig
//
//*************************************************************************

void f8070::getSelectedNvConfig(void)
{
    IFeature8070ColorLEDEffects::NvCapability nvCap = (IFeature8070ColorLEDEffects::NvCapability)(_nvSettingsList->currentData().toInt());
    unsigned char value, param1, param2;
    if (feature8070->GetNvConfig(nvCap, &value, &param1, &param2))
    {
        _nvValue->setText(QString("%1").arg(value));
        _nvParam1->setText(QString("%1").arg(param1));
        _nvParam2->setText(QString("%1").arg(param2));
    }
    else
    {
        notifications->append("GetNvConfig failed (or not supported)");
    }
}


//*************************************************************************
//
// f8070::setSelectedNvConfig
//
//*************************************************************************

void f8070::setSelectedNvConfig(void)
{
    IFeature8070ColorLEDEffects::NvCapability nvCap = (IFeature8070ColorLEDEffects::NvCapability)(_nvSettingsList->currentData().toInt());
    unsigned char value = (unsigned char)_nvValue->text().toUInt();
    unsigned char param1 = (unsigned char)_nvParam1->text().toUInt();
    unsigned char param2 = (unsigned char)_nvParam2->text().toUInt();

    if (!feature8070->SetNvConfig(nvCap, value, param1, param2))
    {
        notifications->append("SetNvConfig failed (or not supported)");
    }
}


//*************************************************************************
//
// f8070::onGetCurrentEffectSettings
//
//*************************************************************************

void f8070::onGetCurrentEffectSettings(void)
{
    vector<Byte> effectParams;
    int zoneEffectIndex = 0;
    if (feature8070->GetEffectSettings(_zoneList->currentIndex(), zoneEffectIndex, effectParams))
    {
        // THERE IS A BUG..
        // Doc says byte0 is the zone index
        // But it seems to return
        // Byte0: Zone index
        // Byte1: Zone effect index
        // Byte2+: Parameters
        zoneEffectIndex = effectParams[0];
        if (zoneEffectIndex > _effectsList->count())
        {
            return;
        }

        _effectsList->setCurrentIndex(zoneEffectIndex);
        for (int i = 1; i <= 10; i++)
        {
            QString val = QString("0x%1").arg((int)effectParams[i], 0, 16);
            _params[i-1]->setText(val);
        }
    }

}

