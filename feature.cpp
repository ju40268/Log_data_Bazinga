#include <QTextEdit>
#include <sstream>
#include <iomanip>
#include <map>
#include <string>
#include "stringbuilder.h"

#include "feature.h"

#include "f0000.h"
#include "f0003.h"
#include "f0005.h"
#include "f0007.h"
#include "f0020.h"
#include "f00c0.h"
#include "f00c1.h"
#include "f00c2.h"
#include "f00d0.h"
#include "f1000.h"
#include "f1001.h"
#include "f1300.h"
#include "f1800.h"
#include "f1802.h"
#include "f1805.h"
#include "f1806.h"
#include "f1814.h"
#include "f1815.h"
#include "f1981.h"
#include "f1A00.h"
#include "f1A01.h"
#include "f1A30.h"
#include "f1B00.h"
#include "f1B03.h"
#include "f1B04.h"
#include "f1BC0.h"
#include "f1C00.h"
#include "f1D4B.h"
#include "f1E00.h"
#include "f1F1F.h"
#include "f1F20.h"
#include "f2005.h"
#include "f2110.h"
#include "f2121.h"
#include "f2130.h"
#include "f2201.h"
#include "f2205.h"
#include "f2230.h"
#include "f2240.h"
#include "f2400.h"
#include "f40A2.h"
#include "f40A3.h"
#include "f4220.h"
#include "f4521.h"
#include "f4522.h"
#include "f4530.h"
#include "f4531.h"
#include "f4540.h"
#include "f4600.h"
#include "f6010.h"
#include "f6012.h"
#include "f6100.h"
#include "f6500.h"
#include "f6501.h"
#include "f8010.h"
#include "f8020.h"
#include "f8030.h"
#include "f8060.h"
#include "f8070.h"
#include "f8080.h"
#include "f8100.h"
#include "f8110.h"
#include "f8111.h"
#include "f8120.h"
#include "f8123.h"
#include "f8300.h"
#include "f8310.h"
#include "xlog/xlog.h"

using std::string;

string feature_name(int number);

Feature::Feature(Device *dev, QStackedLayout *sl, shared_ptr<devio::IFeature> f, QLabel* errlabel) :
    FeatureBase(dev,sl,errlabel),
    feature(f)
{
    std::ostringstream s;
    s << std::hex << std::setfill('0') << std::setw(2) << f->feature_index() << " - " << std::setw(4) << f->id() << " - " << feature_name(f->id());
    setText(s.str().c_str());

    // subscribe for lost settings reports
    if (feature->is<devio::IFeature0020ConfigChange>())
    {
        Info("tracking 0x0020 for lost settings");
        auto f20 = feature->as<devio::IFeature0020ConfigChange>();
        if (f20)
        {
            Info("subscribing for lost settings");
            subscribe(f20);
        }
    }
}

void Feature::onCookieSet(unsigned int swid, unsigned short cookie)
{
    Info("Feature::onCookieSet swid=%d cookie=%d", swid, cookie);

    if (cookie == 0)
    {
        Info("Feature::onCookieSet - settings were lost");
        if (device)
        {
            device->settings_lost();
        }
    }
}

void Feature::settings_lost()
{
    if (feature->is<devio::IFeature6100TouchPadRawXY>())
    {
        f6100* feature = dynamic_cast<f6100*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }
    }

    if (feature->is<devio::IFeature1B03SpecialKeys>())
    {
        f1B03* feature = dynamic_cast<f1B03*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }
    }

    if (feature->is<devio::IFeature1B04SpecialKeys>())
    {
        f1B04* feature = dynamic_cast<f1B04*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }
    }

    if (feature->is<devio::IFeature2005ButtonSwapCancel>())
    {
        f2005* feature = dynamic_cast<f2005*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }
    }

    if (feature->is<devio::IFeature2110SmartShift>())
    {
        f2110* feature = dynamic_cast<f2110*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }
    }

    if (feature->id() == 0x2121)
    {
        f2121* feature = dynamic_cast<f2121*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }
    }

    if (feature->id() == 0x1A30)
    {
        f1A30* feature = dynamic_cast<f1A30*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }
    }

    if (feature->is<devio::IFeature40A2FnInversion>())
    {
        f40A2* feature = dynamic_cast<f40A2*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }
    }

    if (feature->is<devio::IFeature40A3FnInversion>())
    {
        f40A3* feature = dynamic_cast<f40A3*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }
    }

    if (feature->is<devio::IFeature4521DisableKeys>())
    {
        f4521* feature = dynamic_cast<f4521*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }
    }



    if (feature->is<devio::IFeature6501Gestures>())
    {
        f6501* feature = dynamic_cast<f6501*>(ui);
        if (feature)
        {
            feature->settings_lost();
        }

    }
}

void Feature::activate()
{
    if (!ui)
    {
        stringbuilder s;
        s << "Feature " << std::hex << feature->id();

        if (feature->version())
        {
            s << " v" << feature->version();
        }

        s << " - " << feature_name(feature->id()) << "  ";

        if (feature->obsolete()) s << " (obsolete)";
        if (feature->sw_hidden()) s << " (hidden from sw)";
        if (feature->eng_hidden()) s << " (hidden from eng)";
        s << "\n";

        description = s;

        if (feature->is<devio::IFeature0000Root>())
        {
            ui = new f0000(feature->as<devio::IFeature0000Root>(),this);
        }
        else if (feature->is<devio::IFeature0003FirmwareInfo>())
        {
            ui = new f0003(feature->as<devio::IFeature0003FirmwareInfo>(),this);
        }
        else if (feature->is<devio::IFeature0005DeviceNameType>())
        {
            ui = new f0005(feature->as<devio::IFeature0005DeviceNameType>(),this);
        }
        else if (feature->is<devio::IFeature0007DeviceFriendlyName>())
        {
            ui = new f0007(feature->as<devio::IFeature0007DeviceFriendlyName>(), this);
        }
        else if (feature->is<devio::IFeature0020ConfigChange>())
        {
            ui = new f0020(feature->as<devio::IFeature0020ConfigChange>(),this);
        }
        else if (feature->is<devio::IFeature00c0DFUControl>())
        {
            ui = new f00c0(feature->as<devio::IFeature00c0DFUControl>(),this);
        }
        else if (feature->is<devio::IFeature00c1DFUControl>())
        {
            ui = new f00c1(feature->as<devio::IFeature00c1DFUControl>(),this);
        }
        else if (feature->is<devio::IFeature00c2DFUControl>())
        {
            ui = new f00c2(feature->as<devio::IFeature00c2DFUControl>(), this);
        }
        else if (feature->is<devio::IFeature00d0DFU>())
        {
            ui = new f00d0(feature->as<devio::IFeature00d0DFU>(),this);
        }
        else if (feature->is<devio::IFeature1000BatteryUnifiedLevelStatus>())
        {
            ui = new f1000(feature->as<devio::IFeature1000BatteryUnifiedLevelStatus>(),this);
        }
        else if (feature->is<devio::IFeature1001BatteryVoltage>())
        {
            ui = new f1001(feature->as<devio::IFeature1001BatteryVoltage>(), this);
        }
        else if (feature->is<devio::IFeature1300LEDControl>())
        {
            ui = new f1300(feature->as<devio::IFeature1300LEDControl>(), this);
        }
        else if (feature->is<devio::IFeature1800GenericTest>())
        {
            ui = new f1800(feature->as<devio::IFeature1800GenericTest>(), this);
        }
        else if (feature->is<devio::IFeature1802DeviceReset>())
        {
            ui = new f1802(feature->as < devio::IFeature1802DeviceReset>(), this);
        }
        else if (feature->is<devio::IFeature1806ConfigurableDeviceProperties>())
        {
            ui = new f1806(feature->as<devio::IFeature1806ConfigurableDeviceProperties>());
        }
        else if (feature->is<devio::IFeature1814ChangeHost>())
        {
            ui = new f1814(feature->as<devio::IFeature1814ChangeHost>(), this);
        }
        else if (feature->is<devio::IFeature1815HostsInfos>())
        {
            ui = new f1815(feature->as<devio::IFeature1815HostsInfos>(), this);
        }
        else if (feature->is<devio::IFeature1981KeyboardBacklight>())
        {
            ui = new f1981(feature->as<devio::IFeature1981KeyboardBacklight>(),this);
        }
        else if (feature->id() == 0x1A30)
        {
            ui = new f1A30(feature, this);
        }
        else if (feature->is<devio::IFeature1B00SpecialKeys>())
        {
            ui = new f1B00(feature->as<devio::IFeature1B00SpecialKeys>(),this);
        }
        else if (feature->is<devio::IFeature1B03SpecialKeys>())
        {
            ui = new f1B03(feature->as<devio::IFeature1B03SpecialKeys>(),this);
        }
        else if (feature->is<devio::IFeature1B04SpecialKeys>())
        {
            ui = new f1B04(feature->as<devio::IFeature1B04SpecialKeys>(),this);
        }
        else if (feature->is<devio::IFeature1BC0ReportHIDUsages>())
        {
            ui = new f1BC0(feature->as<devio::IFeature1BC0ReportHIDUsages>());
        }
        else if (feature->is<devio::IFeature1C00PersistentRemappableAction>())
        {
            ui = new f1C00(feature->as<devio::IFeature1C00PersistentRemappableAction>());
        }
        else if (feature->is<devio::IFeature1D4BWirelessStatus>())
        {
            ui = new f1D4B(feature->as<devio::IFeature1D4BWirelessStatus>(), this);
        }
        else if (feature->is<devio::IFeature1E00EnableHiddenFeatures>())
        {
            ui = new f1E00(feature->as<devio::IFeature1E00EnableHiddenFeatures>(), this);
        }
        else if (feature->is<devio::IFeature1F1FFirmwareProperties>())
        {
            ui = new f1F1F(feature->as<devio::IFeature1F1FFirmwareProperties>(), this);
        }
        else if (feature->is<devio::IFeature1F20ADCMeasurement>())
        {
            ui = new f1F20(feature->as<devio::IFeature1F20ADCMeasurement>(), this);
        }
        else if (feature->is<devio::IFeature2005ButtonSwapCancel>())
        {
            ui = new f2005(feature->as<devio::IFeature2005ButtonSwapCancel>(),this);
        }
        else if (feature->is<devio::IFeature2110SmartShift>())
        {
            ui = new f2110(feature->as<devio::IFeature2110SmartShift>(),this);
        }
        else if (feature->is<devio::IFeature2121HiResWheel>())
        {
            ui = new f2121(feature->as<devio::IFeature2121HiResWheel>(),this);
        }
        else if (feature->is<devio::IFeature2130RatchetWheel>())
        {
            ui = new f2130(feature->as<devio::IFeature2130RatchetWheel>(),this);
        }
        else if (feature->is<devio::IFeature2201AdjustableDPI>())
        {
            ui = new f2201(feature->as<devio::IFeature2201AdjustableDPI>(),this);
        }
        else if (feature->is<devio::IFeature2205PointerMotionScaling>())
        {
            ui = new f2205(feature->as<devio::IFeature2205PointerMotionScaling>(),this);
        }
        else if (feature->is<devio::IFeature2230AngleSnapping>())
        {
            ui = new f2230(feature->as<devio::IFeature2230AngleSnapping>(),this);
        }
        else if (feature->is<devio::IFeature2240SurfaceTuning>())
        {
            ui = new f2240(feature->as<devio::IFeature2240SurfaceTuning>(),this);
        }
        else if (feature->is<devio::IFeature2400HybridTracking>())
        {
            ui = new f2400(feature->as<devio::IFeature2400HybridTracking>(),this);
        }
        else if (feature->is<devio::IFeature40A2FnInversion>())
        {
            ui = new f40A2(feature->as<devio::IFeature40A2FnInversion>(),this);
        }
        else if (feature->is<devio::IFeature40A3FnInversion>())
        {
            ui = new f40A3(feature->as<devio::IFeature40A3FnInversion>(), this);
        }
        else if (feature->is<devio::IFeature4220LockKeyState>())
        {
            ui = new f4220(feature->as<devio::IFeature4220LockKeyState>(),this);
        }
        else if (feature->is<devio::IFeature4521DisableKeys>())
        {
            ui = new f4521(feature->as<devio::IFeature4521DisableKeys>(),this);
        }
        else if (feature->is<devio::IFeature4522DKU>())
        {
            ui = new f4522(feature->as<devio::IFeature4522DKU>(), this);
        }
        else if (feature->is<devio::IFeature4530DualPlatform>())
        {
            ui = new f4530(feature->as<devio::IFeature4530DualPlatform>(),this);
        }
		else if (feature->is<devio::IFeature4531MultiPlatform>())
		{
			ui = new f4531(feature->as<devio::IFeature4531MultiPlatform>(), this);
		}
        else if (feature->is<devio::IFeature4540KBLayout>())
        {
            ui = new f4540(feature->as<devio::IFeature4540KBLayout>(),this);
        }
        else if (feature->is<devio::IFeature4600Crown>())
        {
            ui = new f4600(feature->as<devio::IFeature4600Crown>(),this);
        }
        else if (feature->is<devio::IFeature6010Gestures>())
        {
            ui = new f6010(feature->as<devio::IFeature6010Gestures>(),this);
        }
        else if (feature->is<devio::IFeature6012Gestures>())
        {
            ui = new f6012(feature->as<devio::IFeature6012Gestures>(),this);
        }
        else if (feature->is<devio::IFeature6100TouchPadRawXY>())
        {
            ui = new f6100(feature->as<devio::IFeature6100TouchPadRawXY>(),this);
        }
        else if (feature->is<devio::IFeature6500Gestures>())
        {
            ui = new f6500(feature->as<devio::IFeature6500Gestures>(),this);
        }
        else if (feature->is<devio::IFeature6501Gestures>())
        {
            ui = new f6501(feature->as<devio::IFeature6501Gestures>(),this);
        }
        else if(feature->is<devio::IFeature8010Gkey>())
        {
            ui = new f8010(feature->as<devio::IFeature8010Gkey>(), this);
        }
        else if(feature->is<devio::IFeature8020Mkeys>())
        {
            ui = new f8020(feature->as<devio::IFeature8020Mkeys>(), this);
        }
        else if(feature->is<devio::IFeature8030MR>())
        {
            ui = new f8030(feature->as<devio::IFeature8030MR>(), this);
        }
        else if (feature->is<devio::IFeature8060ReportRate>())
        {
            ui = new f8060(feature->as<devio::IFeature8060ReportRate>(),this);
        }
        else if (feature->is<devio::IFeature8070ColorLEDEffects>())
        {
            ui = new f8070(feature->as<devio::IFeature8070ColorLEDEffects>(),this);
        }
        else if (feature->is<devio::IFeature8080PerKeyLighting>())
        {
            ui = new f8080(feature->as<devio::IFeature8080PerKeyLighting>(),this);
        }
        else if (feature->is<devio::IFeature8100OnboardProfiles>())
        {
            ui = new f8100(feature->as<devio::IFeature8100OnboardProfiles>(),this);
        }
        else if (feature->is<devio::IFeature8110MouseButtonSpy>())
        {
            ui = new f8110(feature->as<devio::IFeature8110MouseButtonSpy>(),this);
        }
        else if (feature->is<devio::IFeature8111LatencyMonitoring>())
        {
            ui = new f8111(feature->as<devio::IFeature8111LatencyMonitoring>(), this);
        }
        else if (feature->is<devio::IFeature8120GamingAttachments>())
        {
            ui = new f8120(feature->as<devio::IFeature8120GamingAttachments>(), this);
        }
        else if (feature->is<devio::IFeature8123ForceFeedback>())
        {
            ui = new f8123(feature->as<devio::IFeature8123ForceFeedback>(), this);
        }
        else if (feature->is<devio::IFeature8300Sidetone>())
        {
            ui = new f8300(feature->as<devio::IFeature8300Sidetone>(), this);
        }
        else if (feature->is<devio::IFeature8310Equalizer>())
        {
            ui = new f8310(feature->as<devio::IFeature8310Equalizer>(), this);
        }
        else if (feature->is<devio::IFeature1A00PresenterControl>())
        {
            ui = new f1A00(feature->as<devio::IFeature1A00PresenterControl>(), this);
        }
        else if (feature->is<devio::IFeature1A013DSensor>())
        {
            ui = new f1A01(feature->as<devio::IFeature1A013DSensor>(), this);
        }
        else if (feature->is<devio::IFeature1805OOBState>())
        {
            ui = new f1805(feature->as<devio::IFeature1805OOBState>(), this);
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

string Feature::feature_name(int number)
{
    using std::map;
    static map<int,string> feature_names;

    if (feature_names.empty())
    {
        feature_names[0x0000] = "Root";
        feature_names[0x0001] = "FeatureSet";
        feature_names[0x0002] = "FeatureInfo";
        feature_names[0x0003] = "FirmwareInfo";
        feature_names[0x0005] = "DeviceNameType";
        feature_names[0x0007] = "DeviceFriendlyName";
        feature_names[0x0020] = "ConfigChange";
        feature_names[0x00c0] = "OldDfuControl";
        feature_names[0x00c1] = "DfuControl";
        feature_names[0x00c2] = "SecureDfu";
        feature_names[0x00d0] = "DeviceDfu";
        feature_names[0x0100] = "eQuadS4ReceiverControl";
        feature_names[0x0101] = "eQuadS4ReceiverInfo";
        feature_names[0x1000] = "BatteryLevelStatus";
        feature_names[0x1001] = "BatteryVoltage";
        feature_names[0x1300] = "LEDControl";
        feature_names[0x1800] = "GenericTest";
        feature_names[0x1801] = "ManufacturingMode";
        feature_names[0x1802] = "DeviceReset";
        feature_names[0x1803] = "GpioAccess";
        feature_names[0x1804] = "SetFwVersion";
        feature_names[0x1805] = "SetOobState";
        feature_names[0x1806] = "ConfigurableDeviceProperties";
        feature_names[0x1808] = "SetAesEncryptionKey";
        feature_names[0x1810] = "eQuadPairing";
        feature_names[0x1811] = "eQuadPairingEncrypted";
        feature_names[0x1812] = "eQuadPairMultihost";
        feature_names[0x1813] = "eQuadPairMultiEncrypt";
        feature_names[0x1814] = "ChangeHost";
        feature_names[0x1815] = "HostsInfos";
        feature_names[0x1820] = "SimulatedKeystroke";
        feature_names[0x1830] = "PowerModes";
        feature_names[0x1840] = "PerTest";
        feature_names[0x1850] = "SqualShutter";
        feature_names[0x1860] = "AdcValue";
        feature_names[0x1861] = "BatteryCalibration";
        feature_names[0x1868] = "TestBattery";
        feature_names[0x1870] = "SensorTest";
        feature_names[0x1880] = "NvmAccess";
        feature_names[0x1884] = "TdeCheckStatus";
        feature_names[0x1890] = "RfTest";
        feature_names[0x18A0] = "LedTest";
        feature_names[0x18A1] = "LedTest";
        feature_names[0x18B0] = "MonitorMode";
        feature_names[0x18C0] = "RollerTest";
        feature_names[0x18D0] = "TouchpadPowerModesTest";
        feature_names[0x18E0] = "GothardSetVoltage";
        feature_names[0x18E1] = "GothardGetCurrent";
        feature_names[0x18E2] = "SynTpadF05";
        feature_names[0x18E3] = "SynTpadF09";
        feature_names[0x18E4] = "BashBM250Accelerometer";
        feature_names[0x18E5] = "s7020Test";
        feature_names[0x18E6] = "Cytra112003Test";
        feature_names[0x18E7] = "BeringLCD";
        feature_names[0x1981] = "KeyboardBacklight";
        feature_names[0x1A00] = "PresenterControl";
        feature_names[0x1A01] = "3DSensor";
        feature_names[0x1A10] = "LightLevel";
        feature_names[0x1A20] = "AlsCalibration";
        feature_names[0x1A30] = "ProximityDetection";
        feature_names[0x1A40] = "GetSetDate";
        feature_names[0x1A41] = "GetSetTime";
        feature_names[0x1B00] = "SpecialKeysMSEButtons";
        feature_names[0x1B01] = "SpecialKeysMSEButtons";
        feature_names[0x1B02] = "SpecialKeysMSEButtons";
        feature_names[0x1B03] = "SpecialKeysMSEButtons";
        feature_names[0x1B04] = "SpecialKeys";
        feature_names[0x1BC0] = "ReportHIDUsages";
        feature_names[0x1C00] = "PersistentRemappableAction";
        feature_names[0x1D4B] = "WirelessDeviceStatus";
        feature_names[0x1DF0] = "OtpConnectionState";
        feature_names[0x1DF3] = "eQuadDjDebugInfo";
        feature_names[0x1E00] = "EnableHiddenFeatures";
        feature_names[0x1E20] = "SpiLowLevelAccess";
        feature_names[0x1E21] = "SpiReadWriteAccess";
        feature_names[0x1E80] = "MemoryAccess";
        feature_names[0x1E90] = "OtpMemoryAccess";
        feature_names[0x1EA0] = "Jump";
        feature_names[0x1EA1] = "FuseBytes";
        feature_names[0x1EB0] = "TdeNvmAccess";
        feature_names[0x1F01] = "BeijingTouchpadAccess";
        feature_names[0x1F02] = "DubaiProximityDebug";
        feature_names[0x1F03] = "XYSensorAccess";
        feature_names[0x1F04] = "TouchpadDfu";
        feature_names[0x1F05] = "QuarkDebug";
        feature_names[0x1F06] = "QuarkDeviceConfig";
        feature_names[0x1F07] = "BeijingTouchpadAccess";
        feature_names[0x1F09] = "CancunTouchpadDFU";
        feature_names[0x1F1F] = "FirmwareProperties";
        feature_names[0x1F20] = "ADCMeasurement";
        feature_names[0x2001] = "SwapLeftRightButton";
        feature_names[0x2005] = "ButtonSwapCancel";
        feature_names[0x2010] = "SwapMouseButtons";
        feature_names[0x2100] = "VerticalScrolling";
        feature_names[0x2101] = "EnhancedVertScrolling";
        feature_names[0x2110] = "SmartShift";
        feature_names[0x2120] = "HighResScrolling";
        feature_names[0x2121] = "HiResWheel";
        feature_names[0x2130] = "LowResWheel";
        feature_names[0x2200] = "MousePointer";
        feature_names[0x2201] = "AdjustableDpi";
        feature_names[0x2205] = "PointerMotionScaling";
        feature_names[0x2210] = "Obsolete";
        feature_names[0x2220] = "SensorAngleTunability";
        feature_names[0x2230] = "SensorAngleSnapping";
        feature_names[0x2240] = "SurfaceTuning";
        feature_names[0x2300] = "TiltWheel";
        feature_names[0x2400] = "HybridTracking";
        feature_names[0x2500] = "CustomBallistic";
        feature_names[0x2510] = "ProfileManagement";
        feature_names[0x40A0] = "FnInversion";
        feature_names[0x40A2] = "FnInversion";
        feature_names[0x40A3] = "FnInversion";
        feature_names[0x4100] = "Encryption";
        feature_names[0x4180] = "Backlight";
        feature_names[0x4210] = "KeyCounter";
        feature_names[0x4220] = "LockKeyState";
        feature_names[0x4301] = "SolarKeyboardDashboard";
        feature_names[0x4302] = "SolarKeyboardDashboard";
        feature_names[0x4400] = "LCD";
        feature_names[0x4500] = "Configurations";
        feature_names[0x4510] = "Configurations";
        feature_names[0x4520] = "KeyboardLayout";
        feature_names[0x4521] = "DisableKeys";
        feature_names[0x4522] = "DisableKeysByUsage";
        feature_names[0x4530] = "Dual Platform";
		feature_names[0x4531] = "Multi Platform";		
        feature_names[0x4540] = "Keyboard International Layout";
        feature_names[0x4600] = "Crown";
        feature_names[0x5500] = "Slider";
        feature_names[0x6000] = "Touchpad";
        feature_names[0x6010] = "TouchpadFwItems";
        feature_names[0x6011] = "TouchpadSwItems";
        feature_names[0x6012] = "TouchpadWin8FwItems";
        feature_names[0x6020] = "TouchpadTapEnable";
        feature_names[0x6021] = "TapEnableExtended";
        feature_names[0x6030] = "CursorBallistics";
        feature_names[0x6040] = "ResolutionDivider";
        feature_names[0x6100] = "TouchpadRawXY";
        feature_names[0x6110] = "TouchMouseRawTouch";
        feature_names[0x6120] = "BtTouchMouseSettings";
        feature_names[0x6401] = "TouchPadGestures";
        feature_names[0x6500] = "Gestures";
        feature_names[0x6501] = "Gestures";
        feature_names[0x7001] = "SendBrokenArrow";
        feature_names[0x8000] = "Gaming";
        feature_names[0x8010] = "GKeys";
        feature_names[0x8020] = "MKeys";
        feature_names[0x8030] = "MacroRecordKey";
        feature_names[0x8040] = "Brightness";
        feature_names[0x8050] = "LedIndication";
        feature_names[0x8060] = "ReportRate";
        feature_names[0x8070] = "ColorLEDEffects";
        feature_names[0x8080] = "PerKeyLighting";
        feature_names[0x8100] = "OnboardProfiles";
        feature_names[0x8110] = "MouseButtonSpy";
        feature_names[0x8111] = "LatencyMonitoring";
        feature_names[0x8120] = "GamingAttachments";
        feature_names[0x8123] = "ForceFeedback";
        feature_names[0x8300] = "Sidetone";
        feature_names[0x8310] = "Equalizer";
        feature_names[0x9000] = "TestAg8020";
        feature_names[0x9200] = "TestLis3mdl";
        feature_names[0xF001] = "FsrThresholdAccess";
        feature_names[0xFFFD] = "Test";
        feature_names[0xFFFE] = "Test";
    }

    return feature_names[number];
}

// Define the set of features used in the BazingaDFU build
bool Feature::isBazingaDFUFeature(const int feature_id)
{
    switch(feature_id)
    {
        // Core
    case 0x0000:
    case 0x0001:
        // Version
    case 0x0003:
        // Name
    case 0x0005:
    case 0x0007:
        // DFU
    case 0x00c0:
    case 0x00c1:
    case 0x00c2:
    case 0x00d0:
        // Battery Status
    case 0x1000:
    case 0x1001:
        return true;
    default:
        return false;
    }
}
