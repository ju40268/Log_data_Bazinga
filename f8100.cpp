#include "feature.h"
#include "f8100.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include <QMenu>
#include <QGroupBox>
#include <QValidator>

using devio::Delivery;


f8100::f8100(shared_ptr<devio::IFeature8100OnboardProfiles> f, Feature* base):
    QWidget(),
    feature8100(f),
    baseFeature(base),
    notifications(NULL),
    uiOnboardModes(NULL),
    isOnboardMode(true),
    activeProfile(devio::Profile_1)
{
    createUI();
    refreshData();
    refreshUI();

    subscribe(feature8100,Delivery::Immediate);
}


void f8100::createUI(void)
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    setLayout(vLayout);

    {
        QGroupBox* group = new QGroupBox("Profile Mode");
        QHBoxLayout* dataLayout = new QHBoxLayout;
        group->setLayout(dataLayout);

        uiOnboardModes = new QComboBox;
        uiOnboardModes->addItem("Onboard Mode");
        uiOnboardModes->addItem("Host Mode");
        connect(uiOnboardModes, SIGNAL(currentIndexChanged(int)), this, SLOT(onOnboardModeChanged(int)));
        dataLayout->addWidget(uiOnboardModes);
        
        uiOnboardProfiles = new QComboBox;

        // Add the standard profiles
        uiOnboardProfiles->addItem("Profile 1", devio::Profile_1);
        uiOnboardProfiles->addItem("Profile 2", devio::Profile_2);
        uiOnboardProfiles->addItem("Profile 3", devio::Profile_3);
        uiOnboardProfiles->addItem("Profile 4", devio::Profile_4);
        uiOnboardProfiles->addItem("Profile 5", devio::Profile_5);
        uiOnboardProfiles->addItem("Profile 1 (ROM)", devio::Profile_1_ROM);
        uiOnboardProfiles->addItem("Profile 2 (ROM)", devio::Profile_2_ROM);
        uiOnboardProfiles->addItem("Profile 3 (ROM)", devio::Profile_3_ROM);
        connect(uiOnboardProfiles, SIGNAL(currentIndexChanged(int)), this, SLOT(onOnboardProfileChanged(int)));
        dataLayout->addWidget(uiOnboardProfiles);

        vLayout->addWidget(group);
    }

    {
        QGroupBox* profileGroup = new QGroupBox("Profile Read / Write");
        QHBoxLayout* buttonLayout = new QHBoxLayout;
        profileGroup->setLayout(buttonLayout);
        QPushButton* readProfiles = new QPushButton("Read All Profiles");
        connect(readProfiles, SIGNAL(clicked(void)), this, SLOT(onReadAllProfiles(void)));
        buttonLayout->addWidget(readProfiles);
        QPushButton* writeProfiles = new QPushButton("Write ROM to Profiles");
        connect(writeProfiles, SIGNAL(clicked(void)), this, SLOT(onWriteROMProfiles(void)));
        buttonLayout->addWidget(writeProfiles);

        vLayout->addWidget(profileGroup);
    }
    {
    /* TBD
        QGroupBox* group = new QGroupBox("Profile Directory");
        QHBoxLayout* dataLayout = new QHBoxLayout;
        group->setLayout(dataLayout);
        
        QPushButton* readProfileDir = new QPushButton("Read Profiles Directory");
        //connect(readProfileDir, SIGNAL(pressed()), this, SLOT(onReadProfileDir()));
        dataLayout->addWidget(readProfileDir);
        
        vLayout->addWidget(group);
    */
    }

    // Notifications
    {
        notifications = new QTextEdit;
        notifications->setReadOnly(true);
        notifications->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(notifications, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onNotificationContextMenu(const QPoint&)));
        vLayout->addWidget(notifications);
    }
}

void f8100::notifyText(const QString &text, bool insertCRLF)
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

void f8100::onNotificationContextMenu(const QPoint &pos)
{
    QMenu popup(this);
    popup.addAction("Clear", this, SLOT(onClearNotifications()));
    popup.exec(notifications->mapToGlobal(pos));
}

void f8100::onClearNotifications(void)
{
    notifications->clear();
}

void f8100::onOnboardModeChanged(int index)
{
    switch(index)
    {
        case 0:
            if (!feature8100->SetOnboardMode(true))
            {
                notifyText("ERROR: Unable to set device into onboard mode");
            }
            else
            {
                notifyText("Device is now in onboard mode");
                uiOnboardProfiles->setEnabled(true);
            }
            break;
        case 1:
            if (!feature8100->SetOnboardMode(false))
            {
                notifyText("ERROR: Unable to set device into host mode");
            }
            else
            {
                notifyText("Device is now in host mode");
                uiOnboardProfiles->setEnabled(false);
            }
            break;
        default:
            break;
    }
}

void f8100::onOnboardProfileChanged(int index)
{
    // Apply it to the device
    uiOnboardProfiles->blockSignals(true);
    ProfileId profileId = (ProfileId)(uiOnboardProfiles->itemData(index).toInt());
    if (!feature8100->SetActiveProfile(profileId))
    {
        notifyText(QString("ERROR: SetActiveProfile failed for profile %1").arg(profileId));
    }
    uiOnboardProfiles->blockSignals(false);
}

void f8100::refreshData(void)
{
    // info
    if (!feature8100->GetOnboardProfilesInfo(onboardProfilesInfo))
    {
        notifyText("ERROR: GetOnboardMode failed");
    }
    else
    {
        notifyText("Onboard Profiles Summary:");
        notifyText(QString("  Memory model id: %1").arg(onboardProfilesInfo.memoryModelId));
        notifyText(QString("  Profile format id: %1").arg(onboardProfilesInfo.profileFormatId));
        notifyText(QString("  Macro format id: %1").arg(onboardProfilesInfo.macroFormatId));
        notifyText(QString("  Profile count: %1").arg(onboardProfilesInfo.profileCount));
        notifyText(QString("  Profile OOB count: %1").arg(onboardProfilesInfo.profileCountOOB));
        notifyText(QString("  Button count: %1").arg(onboardProfilesInfo.buttonCount));
        notifyText(QString("  sector count: %1").arg(onboardProfilesInfo.sectorCount));
        notifyText(QString("  sector size: %1").arg(onboardProfilesInfo.sectorSize));
        notifyText(QString("  supports GShift: %1").arg(onboardProfilesInfo.supportsGShift));
        notifyText(QString("  supports DPI Shift: %1").arg(onboardProfilesInfo.supportsDPIShift));
    }
    
    // Current mode
    isOnboardMode = true;
    if (!feature8100->IsOnboardMode(isOnboardMode))
    {
        notifyText("ERROR: GetOnboardMode failed");
    }
}

void f8100::refreshUI(void)
{
    uiOnboardModes->blockSignals(true);
    uiOnboardProfiles->blockSignals(true);
    
    uiOnboardModes->setCurrentIndex(isOnboardMode ? 0 : 1);
    uiOnboardProfiles->setEnabled(isOnboardMode);

    if (isOnboardMode)
    {
        if (!feature8100->GetActiveProfile(activeProfile))
        {
            notifyText("ERROR: GetActiveProfile failed");            
        }
        else
        {
            int profileIndex = uiOnboardProfiles->findData(activeProfile);
            if (-1 != profileIndex)
            {
                uiOnboardProfiles->setCurrentIndex(profileIndex);                
            }
        }
        // TODO: Dynamically add profiles
    }
    uiOnboardModes->blockSignals(false);
    uiOnboardProfiles->blockSignals(false);
}

void f8100::onProfileActivated(enum ProfileId profile_id)
{
    notifyText(QString("Profile 0x%1 activated").arg((int)profile_id, 0, 16));

    activeProfile = profile_id;

    // Show the current profile in the UI
    uiOnboardProfiles->blockSignals(true);
    int profileIndex = uiOnboardProfiles->findData(profile_id);
    if (-1 != profileIndex)
    {
        uiOnboardProfiles->setCurrentIndex(profileIndex);        
    }
    uiOnboardProfiles->blockSignals(false);
}

void f8100::onActiveProfileResolutionChanged(int resoution_index)
{
    notifyText(QString("Resolution changed to index %1").arg(resoution_index));
}

void f8100::onReadProfileDir(void)
{
    // Sector 0
    // 4 bytes per profile
    // 5 profiles
    // Total: 20 bytes
    int needToRead = 20;
    int totalRead = 0;
    int sector = 0;
    int sectorAddr = 0;
    vector<Byte> dataRead;
    while (needToRead > 0)
    {
        vector<Byte> bytes;
        int result = feature8100->ReadData(sector, sectorAddr, bytes);
        if (!result)
        {
            notifyText("ERROR: ReadData failed");
            return;
        }
        else
        {
            totalRead += 16;
            needToRead -= 16;
            sectorAddr += 16;
            // append it
            dataRead.insert(dataRead.end(), bytes.begin(), bytes.end());
        }
    }
    
    notifyText(QString("Read %1 bytes").arg(dataRead.size()));

    // Show 20 bytes
    stringbuilder s;
    for (int i = 0; i < 20; i++)
    {
        s << "0x" << std::hex << (unsigned int)(unsigned char)dataRead[i] << " ";
    }
    notifyText(s);
}

//*************************************************************************
//
// f8100::onReadAllProfiles
//
//*************************************************************************

void f8100::onReadAllProfiles(void)
{
    // info
    if (!feature8100->GetOnboardProfilesInfo(onboardProfilesInfo))
    {
        notifyText("ERROR: GetOnboardMode failed");
        return;
    }

    cFeature8100OnboardProfileHeader* header = new cFeature8100OnboardProfileHeader();
    for (quint16 i = devio::Profile_1_ROM; i < (devio::Profile_1_ROM + onboardProfilesInfo.profileCountOOB); i++)
    {
        // Read the header
        if (!readSectorData(i, 0, (quint8*)header, sizeof(cFeature8100OnboardProfileHeader)))
        {
            continue;
        }

        dumpProfileHeader(i, header);
    }

    for (quint16 i = devio::Profile_1; i < (devio::Profile_1 + onboardProfilesInfo.profileCount); i++)
    {    
        // Read the header
        if (!readSectorData(i, 0, (quint8*)header, sizeof(cFeature8100OnboardProfileHeader)))
        {
            continue;
        }

        dumpProfileHeader(i, header);

    }

    dumpProfileDirectory(readProfileDirectory());
}


//*************************************************************************
//
// f8100::readSectorData
//
//*************************************************************************

bool f8100::readSectorData(quint16 sector, quint16 sectorAddr, quint8* buffer, quint32 length)
{
    // length cannot be zero, or exceed a sector size
    if ((length <= 0) || (length > (quint32)onboardProfilesInfo.sectorSize))
    {
        Q_ASSERT(false);
        return false;
    }
    
    bool status = true;
    quint32 subAddr = sectorAddr;
    quint32 bytesRead = 0;
    while (bytesRead < length)
    {
        vector<Byte> readBytes;
        if (!feature8100->ReadData(sector, sectorAddr + subAddr, readBytes))
        {
            return false;
        }

        // We should always be reading 16 bytes
        if (16 != readBytes.size())
        {
            Q_ASSERT(false);
            return false;
        }

        memcpy(buffer + bytesRead, readBytes.data(), readBytes.size());

        bytesRead += (quint32)readBytes.size();
        subAddr += (quint32)readBytes.size();
    }

    return status;
}

//*************************************************************************
//
// f8100::dumpProfileHeader
//
//*************************************************************************

void f8100::dumpProfileHeader(quint16 sector, cFeature8100OnboardProfileHeader* header)
{
    notifyText("------------------------------------------------------");
    notifyText(QString("Profile: (Sector 0x%1)").arg(QString::number(sector, 16)));
    notifyText(QString("    Profile Name: %1").arg(QString::fromUtf16(header->profileName)));
    notifyText(QString("    Report Rate: %1").arg(header->reportRate));
    notifyText(QString("    Color: r=%1,g=%2,b=%3").arg((quint32)header->ledColor[0]).arg((quint32)header->ledColor[1]).arg((quint32)header->ledColor[2]));
    notifyText(QString("    Power: %1").arg(header->powerMode));
    QString dpiString = QString("    DPI: { ");
    int dpiIndex = 0;
    for (int i = 0; i < 5; i++)
    {
        if (i == header->defaultDpiIndex)
        {
            dpiString.append("*");
        }
        dpiString.append(QString::number((quint32)(header->dpiTable[i])) + ", ");
        dpiIndex++;
    }
    notifyText(dpiString + "}");
    notifyText(QString("    Angle Snapping: %1").arg(header->angleSnapping));
    notifyText(QString("    Write Counter: %1").arg(header->writeCounter));
    QString logoLightingString = QString("    Logo light effect: ");
    for (int a = 0; a < sizeof(header->logoLightEffect); a++)
    {
        logoLightingString.append(" 0x" + QString::number(header->logoLightEffect[a], 16));
    }
    notifyText(logoLightingString);
    QString primaryLightingString = QString("    Primary light effect: ");
    for (int a = 0; a < sizeof(header->primaryLightEffect); a++)
    {
        primaryLightingString.append(" 0x" + QString::number(header->primaryLightEffect[a], 16));
    }
    notifyText(primaryLightingString);
    notifyText("------------------------------------------------------");
}

quint16 CRC_CCIT(quint8 *data, int len)
{
    int crc = 0xFFFF;
    int y;

    for (y = 0; y < len; y++)
    {
        int temp = data[y];
        int b;

        temp <<= 8;

        for (b = 0; b < 8; b++)
        {
            if ((crc ^ temp) & 0x8000)
            {
                crc <<= 1;
                crc = crc ^ 0x1021;
            }
            else
            {
                crc <<= 1;
            }

            temp <<= 1;
        }
    }

    return (unsigned short)(crc & 0xFFFF);
}
         

//*************************************************************************
//
// f8100::onWriteROMProfiles
//
//*************************************************************************

void f8100::onWriteROMProfiles(void)
{
    // info
    if (!feature8100->GetOnboardProfilesInfo(onboardProfilesInfo))
    {
        notifyText("ERROR: GetOnboardMode failed");
        return;
    }

    notifyText("Write Begin");
    QVector<cFeature8100OnboardProfileDirEntry> profileDirectory;// = readProfileDirectory();

    //if (profileDirectory.isEmpty())
    //{
        profileDirectory = createProfileDirectory();
    //}

    notifyText("    Write Profile Directory");
    writeProfileDirectory(profileDirectory);

    int sectorSize = onboardProfilesInfo.sectorSize;

    cFeature8100OnboardProfileHeader* header = new cFeature8100OnboardProfileHeader();
    quint16 profileIndex = devio::Profile_1;
    for (quint16 romIndex = devio::Profile_1_ROM; romIndex < (devio::Profile_1_ROM + onboardProfilesInfo.profileCountOOB); romIndex++)
    {
        // Read the header
        if (!readSectorData(romIndex, 0, (quint8*)header, sizeof(cFeature8100OnboardProfileHeader)))
        {
            continue;
        }

        QVector<quint8> sectorData(sectorSize, 0xff);
        // copy it to the scratch pad sector 
        memcpy(sectorData.data(), (quint8*)header, sizeof(cFeature8100OnboardProfileHeader));
        quint16 crc = CRC_CCIT(sectorData.data(), sectorSize - 2);
        sectorData[sectorSize - 2] = ((quint8)((((quint32)(crc)) >> 8) & 0xff));
        sectorData[sectorSize - 1] = ((quint8)(((quint32)(crc)) & 0xff));

        notifyText(QString("    Write Profile %1").arg(profileIndex));
        writeSectorToDevice(profileIndex, sectorData.data(), sectorSize);

        profileIndex++;
    }


    notifyText("Write Complete");

}

//*************************************************************************
//
// f8100::writeSectorToDevice
//
//*************************************************************************

bool f8100::writeSectorToDevice(quint16 sector, quint8* buffer, quint16 length)
{
    if (length != onboardProfilesInfo.sectorSize)
    {
        Q_ASSERT(false);
        return false;
    }

    // Start the write
    if (!feature8100->BeginWrite(sector, 0, length))
    {
        return false;
    }

    notifyText("Writing" + QString::number(length) + "bytes to sector" + QString::number(sector));

    // Write 16 bytes at a time
    bool status = true;
    quint32 bytesWritten = 0;
    int frameNumber = 0;
    while (bytesWritten < length)
    {
        vector<Byte> dataBlock(16, Byte(0xff));
        Q_ASSERT(sizeof(Byte) == sizeof(quint8));
        memcpy(dataBlock.data(), buffer + bytesWritten, dataBlock.size());

        if (!feature8100->WriteData(dataBlock, frameNumber))
        {
            break;
        }

        bytesWritten += (quint32)dataBlock.size();

        notifyText("Frame:" + QString::number(frameNumber) + "Bytes Written:" + QString::number(bytesWritten) + "/" + QString::number(length));
    }

    feature8100->EndWrite();

    return status;
}

//*************************************************************************
//
// f8100::readProfileDirectory
//
//*************************************************************************

QVector<cFeature8100OnboardProfileDirEntry> f8100::readProfileDirectory(void)
{

    QVector<cFeature8100OnboardProfileDirEntry> profileDirectory;
    profileDirectory.resize(onboardProfilesInfo.profileCount + 1);
    profileDirectory.fill(cFeature8100OnboardProfileDirEntry());


    // Read the sector
    quint8* dataPtr = (quint8*)profileDirectory.data();
    quint32 dataSize = profileDirectory.size() * sizeof(cFeature8100OnboardProfileDirEntry);
    if (!readSectorData(0, 0, dataPtr, dataSize))
    {
        return QVector<cFeature8100OnboardProfileDirEntry>();
    }

    // Ensure the last entry is 0xffff
    if (profileDirectory[onboardProfilesInfo.profileCount].sector() != 0xffff)
    {
        return QVector<cFeature8100OnboardProfileDirEntry>();
    }

    // Ensure each entry is a valid sector
    foreach(const cFeature8100OnboardProfileDirEntry& dirEntry, profileDirectory)
    {
        if (dirEntry.sector() == 0xffff)
        {
            break;
        }

        if (!isValidProfileSector(dirEntry.sector()))
        {
            return QVector<cFeature8100OnboardProfileDirEntry>();
        }

        // Do not allow the profile directory to contain ROM profiles
        // Otherwise, we cannot select the non-ROM profile
        if (isFactoryProfileSector(dirEntry.sector()))
        {
            return QVector<cFeature8100OnboardProfileDirEntry>();
        }
    }

    // If the first entry is not valid or a factory profile, recreate the directory
    if (!isValidProfileSector(profileDirectory[0].sector()))
    {
        return QVector<cFeature8100OnboardProfileDirEntry>();
    }
    
    return profileDirectory;
}

//*************************************************************************
//
// f8100::createProfileDirectory
//
//*************************************************************************

QVector<cFeature8100OnboardProfileDirEntry> f8100::createProfileDirectory(void)
{
    QVector<cFeature8100OnboardProfileDirEntry> profileDirectory;
    profileDirectory.resize(onboardProfilesInfo.profileCount + 1);
    profileDirectory.fill(cFeature8100OnboardProfileDirEntry());

    // NOTE: sectors are initialized to 0xffff to it is already terminated
    int i = 0;
    for (quint16 sector = devio::Profile_1; sector < (devio::Profile_1 + onboardProfilesInfo.profileCountOOB); sector++)
    {
        profileDirectory[i].setSector(sector);
        profileDirectory[i].enabled = 1;
        i++;
    }

    return profileDirectory;
}


//*************************************************************************
//
// f8100::writeProfileDirectory
//
//*************************************************************************

bool f8100::writeProfileDirectory(QVector<cFeature8100OnboardProfileDirEntry> profileDirectory)
{
    if (profileDirectory.empty())
    {
        return false;
    }
    
    int sectorSize = onboardProfilesInfo.sectorSize;
    QVector<quint8> sectorData(sectorSize, 0xff);
    memcpy(sectorData.data(), profileDirectory.data(), profileDirectory.size()*sizeof(profileDirectory[0]));
    quint16 crc = CRC_CCIT(sectorData.data(), sectorSize - 2);
    sectorData[sectorSize - 2] = ((quint8)((((quint32)(crc)) >> 8) & 0xff));
    sectorData[sectorSize - 1] = ((quint8)(((quint32)(crc)) & 0xff));



    return writeSectorToDevice(0, sectorData.data(), (quint16)onboardProfilesInfo.sectorSize);
}


//*************************************************************************
//
// f8100::isValidProfileSector
//
//*************************************************************************

bool f8100::isValidProfileSector(quint16 sector)
{
    // Check low byte
    quint8 profile = (0x00ff & sector);
    quint8 oob = (0xff00 & sector) >> 8;
    return ((profile > 0) && (profile <= onboardProfilesInfo.profileCount) && (oob >= 0) && (oob <= 1));
}

//*************************************************************************
//
// f8100::isFactoryProfileSector
//
//*************************************************************************

bool f8100::isFactoryProfileSector(quint16 sector)
{
    quint8 oob = (0xff00 & sector) >> 8;
    return (oob != 0);
}

//*************************************************************************
//
// f8100::dumpProfileDirectory
//
//*************************************************************************

void f8100::dumpProfileDirectory(QVector<cFeature8100OnboardProfileDirEntry> profileDirectory)
{
    if (profileDirectory.isEmpty())
    {
        notifyText(QString("Invalid Profile Directory"));
    }
    else
    {
    
        // Get the current active profile. If it is a ROM profile, there is something
        // wrong with one of the RW profiles
        ProfileId activeProfileId = ProfileId::Profile_Unknown;
        feature8100->GetActiveProfile(activeProfileId);

        notifyText("Active Profiles (sector 1 profile dir):");
        foreach(const cFeature8100OnboardProfileDirEntry &profileDirEntry, profileDirectory)
        {
            if (profileDirEntry.sector() == 0xffff)
            {
                break;
            }

            notifyText(QString("    sector=0x%1, enabled=%2, reserved=0x%3").arg(QString::number((quint32)profileDirEntry.sector(), 16)).arg((quint32)profileDirEntry.enabled).arg(QString::number((quint32)profileDirEntry.reserved, 16)));
        }
        notifyText(QString("Active Profile Sector: 0x%1").arg(QString::number((quint32)activeProfileId, 16)));
    }
}

