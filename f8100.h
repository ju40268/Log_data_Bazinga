#pragma once

#include <QTextEdit>
#include <QLineEdit>
#include "feature.h"
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <vector>
#include "devio.h"

using devio::IDevice;
using devio::IFeature;
using devio::IFeature8100OnboardProfiles;
using devio::Byte;
using devio::Subscriber;
using devio::ProfileId;
using devio::OnboardProfilesInfo;
using std::vector;


#pragma pack(push,1)

struct cFeature8100OnboardProfileDirEntry
{
    cFeature8100OnboardProfileDirEntry()
    {
        sectorHi = 0xff;
        sectorLo = 0xff;
        enabled = 0;
        reserved = 0;
    }

    // Sector
    quint8 sectorHi;
    quint8 sectorLo;
    // Enabled/Disabled
    quint8 enabled;
    // Reserved
    quint8 reserved;

    void setSector(quint16 s)
    {
        sectorHi = ((quint8)((((quint32)(s)) >> 8) & 0xff));
        sectorLo = ((quint8)(((quint32)(s)) & 0xff));
    }

    quint16 sector(void) const
    {
        return ((sectorHi << 8) | sectorLo);
    }
};

struct cFeature8100OnboardProfileHeader
{
    cFeature8100OnboardProfileHeader()
    {
        reportRate = 0;
        defaultDpiIndex = 0;
        shiftDpiIndex = 0;
        memset(dpiTable, 0, sizeof(dpiTable));
        memset(ledColor, 0, sizeof(dpiTable));
        powerMode = 0;
        angleSnapping = 0;
        memset(reserved, 0, sizeof(reserved));
        memset(buttonMapping, 0, sizeof(buttonMapping));
        memset(gShiftMapping, 0, sizeof(gShiftMapping));
        memset(profileName, 0, sizeof(profileName));
        memset(logoLightEffect, 0, sizeof(logoLightEffect));
        memset(primaryLightEffect, 0, sizeof(primaryLightEffect));
        memset(padding, 0, sizeof(padding));
    }

    cFeature8100OnboardProfileHeader(const cFeature8100OnboardProfileHeader& header)
    {
        *this = header;
    }

    quint8 reportRate;
    quint8 defaultDpiIndex;
    quint8 shiftDpiIndex;
    quint16 dpiTable[5];
    quint8 ledColor[3];
    quint8 powerMode;
    quint8 angleSnapping;
    quint16 writeCounter;
    quint8 reserved[12];
    quint8 buttonMapping[16][4];
    quint8 gShiftMapping[16][4];
    quint16 profileName[24];
    quint8 logoLightEffect[11];
    quint8 primaryLightEffect[11];
    quint8 padding[10];
};

#pragma pack(pop)

class f8100 : public QWidget,
              public Subscriber<devio::IFeature8100OnboardProfiles::Report>
{
    Q_OBJECT
public:
    explicit f8100(shared_ptr<devio::IFeature8100OnboardProfiles> f, Feature* base);
    
    virtual void onProfileActivated(enum ProfileId profile_id);
    virtual void onActiveProfileResolutionChanged(int resoution_index);

protected:
    void createUI(void);
    void refreshData(void);
    void refreshUI(void);
    void notifyText(const QString &text, bool insertCRLF = false);
    bool readSectorData(quint16 sector, quint16 sectorAddr, quint8* buffer, quint32 length);
    void dumpProfileHeader(quint16 sector, cFeature8100OnboardProfileHeader* header);
    bool writeSectorToDevice(quint16 sector, quint8* buffer, quint16 length);
    QVector<cFeature8100OnboardProfileDirEntry> readProfileDirectory(void);
    QVector<cFeature8100OnboardProfileDirEntry> createProfileDirectory(void);
    bool writeProfileDirectory(QVector<cFeature8100OnboardProfileDirEntry> profileDirectory);
    bool isValidProfileSector(quint16 sector);
    bool isFactoryProfileSector(quint16 sector);
    void dumpProfileDirectory(QVector<cFeature8100OnboardProfileDirEntry> profileDirectory);

protected slots:
    void onNotificationContextMenu(const QPoint &pos);
    void onClearNotifications(void);
    void onOnboardModeChanged(int index);
    void onOnboardProfileChanged(int index);
    void onReadProfileDir(void);
    void onReadAllProfiles(void);
    void onWriteROMProfiles(void);
    
private:
    shared_ptr<devio::IFeature8100OnboardProfiles> feature8100;
    Feature* baseFeature;
    QTextEdit* notifications;

    // UI
    QComboBox* uiOnboardModes;
    QComboBox* uiOnboardProfiles;

    // data
    ProfileId activeProfile;
    bool isOnboardMode;
    OnboardProfilesInfo onboardProfilesInfo;
};
