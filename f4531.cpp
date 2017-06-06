#include "feature.h"
#include "f4531.h"
#include <sstream>
#include <iomanip>
#include <QtDebug>
#include <QMenu>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include "stringbuilder.h"

using devio::Delivery;


f4531::f4531(shared_ptr<devio::IFeature4531MultiPlatform> f, Feature* base) :
    QTabWidget(),
    feature4531(f),
    baseFeature(base),	
    m_capabilityMask(0),
    m_numPlatforms(0),
    m_numPlatformDescr(0),
    m_numHosts(0),
    m_currentHost(0),
    m_currentHostPlatform(0)
{
    m_log.reset(new QTextEdit);
    m_eventsLog.reset(new QTextEdit);
    m_featureInfos.reset(new QWidget);

    if (m_log)
    {
        m_log->setPlainText(baseFeature->description);
        m_log->setReadOnly(true);
        m_log->setWordWrapMode(QTextOption::NoWrap);

        m_log->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_log.get(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(logContextualMenu(const QPoint &)));
    }

    if (m_eventsLog)
    {
        m_eventsLog->setPlainText(baseFeature->description);
        m_eventsLog->setReadOnly(true);
        m_eventsLog->setWordWrapMode(QTextOption::NoWrap);
    }

    if (m_featureInfos)
    {
        QVBoxLayout* vLayout = new QVBoxLayout;
        m_featureInfos->setLayout(vLayout);

        QHBoxLayout* getButtonsLayout = new QHBoxLayout;
        if (getButtonsLayout)
        {
            QWidget* getButtons = new QWidget;
            getButtons->setLayout(getButtonsLayout);

            QPushButton* getFeatureInfosButton = new QPushButton("getFeatureInfos");
            connect(getFeatureInfosButton, SIGNAL(clicked(bool)), this, SLOT(getFeatureInfos(void)));

            QPushButton* getPlatformDescriptorButton = new QPushButton("getPlatformDescriptor");
            connect(getPlatformDescriptorButton, SIGNAL(clicked(bool)), this, SLOT(getPlatformDescriptors(void)));

            QPushButton* getHostPlatformButton = new QPushButton("getHostPlatform");
            connect(getHostPlatformButton, SIGNAL(clicked(bool)), this, SLOT(getHostPlatforms(void)));

            getButtonsLayout->addWidget(getFeatureInfosButton);
            getButtonsLayout->addWidget(getPlatformDescriptorButton);
            getButtonsLayout->addWidget(getHostPlatformButton);

            vLayout->addWidget(getButtons);
        }

        QGroupBox* setHostPlatformGroup = new QGroupBox("setHostPlatform");
        QHBoxLayout* setHostPlatformLayout = new QHBoxLayout;
        if (setHostPlatformGroup && setHostPlatformLayout)
        {
            setHostPlatformGroup->setLayout(setHostPlatformLayout);

            QLabel* hostIndexLabel = new QLabel;
            hostIndexLabel->setText("hostIndex:");

            m_hostIndexSpinBox.reset(new QSpinBox);
            m_hostIndexSpinBox->setMinimum(0);
            m_hostIndexSpinBox->setMaximum(0xFF);

            QLabel* platformLabel= new QLabel;
            platformLabel->setText("platform:");

            m_platformSpinBox.reset(new QSpinBox);
            m_platformSpinBox->setMinimum(0);
            m_platformSpinBox->setMaximum(0xFF);

            QPushButton* setHostPlatformButton = new QPushButton("Set");
            connect(setHostPlatformButton, SIGNAL(clicked(bool)), this, SLOT(setHostPlatform(void)));

            setHostPlatformLayout->addWidget(hostIndexLabel);
            setHostPlatformLayout->addWidget(m_hostIndexSpinBox.get());
            setHostPlatformLayout->addWidget(platformLabel);
            setHostPlatformLayout->addWidget(m_platformSpinBox.get());
            setHostPlatformLayout->addWidget(setHostPlatformButton);
            
            vLayout->addWidget(setHostPlatformGroup);
        }

        QGroupBox* logGroup = new QGroupBox("Log");
        if (logGroup)
        {
            QVBoxLayout* logGroupLayout = new QVBoxLayout;
            logGroup->setLayout(logGroupLayout);

            m_currentLog.reset(new QTextEdit);
            if (m_currentLog)
            {
                m_currentLog->setPlainText(baseFeature->description);
                m_currentLog->setReadOnly(true);
                m_currentLog->setWordWrapMode(QTextOption::NoWrap);
                logGroupLayout->addWidget(m_currentLog.get());
            }

            vLayout->addWidget(logGroup);
        }
    }

    _getFeatureInfos();
    _getPlatformDescriptors();
    _getHostPlatforms();


    subscribeToEvents(true);

    addTab(m_featureInfos.get(), "Feature");
    addTab(m_eventsLog.get(), "Events");
    addTab(m_log.get(), "Log");
}


string f4531::desc_capabilityMask(unsigned short capabilityMask)
{
    stringbuilder s;

    s << std::hex << " 0x" << capabilityMask << " ";

    if (m_capabilityMask & devio::x4531CapabilityFlags_OSDetection)
    {
        s << "(osDetection) ";
    }

    if (m_capabilityMask & devio::x4531CapabilityFlags_SetHostPlatform)
    {
        s << "(setHostPlatform) ";
    }

    return s;
}


string f4531::desc_pairingStatus(unsigned short status)
{
    stringbuilder s;

    s << std::hex << " 0x" << status << " ";

    switch (status)
    {
    case 0: s << "(Empty Slot)"; break;
    case 1: s << "(Paired)"; break;
    default: s << "(Unknown)"; break;
    }

    return s;
}

std::string f4531::desc_platform( short platform )
{
    stringbuilder s;

    s << std::hex << "0x" << platform << " ";

    if (platform & devio::x4531PlatformFlags_Windows)
    {
        s << "(Windows)";
    }
    else if (platform & devio::x4531PlatformFlags_WinEmb)
    {
        s << "(Windows Embedded)";
    }
    else if (platform & devio::x4531PlatformFlags_Linux)
    {
        s << "(Linux)";
    }
    else if (platform & devio::x4531PlatformFlags_Chrome)
    {
        s << "(Chrome)";

    }
    else if (platform & devio::x4531PlatformFlags_Android)
    {
        s << "(Android)";
    }
    else if (platform & devio::x4531PlatformFlags_MacOS)
    {
        s << "(MacOS)";

    }
    else if (platform & devio::x4531PlatformFlags_IOS)
    {
        s << "(iOS)";
    }
    else
    {
        s << "(Unknown Platform)";
    }

    return s;
}


string f4531::desc_platformSource(unsigned short platformSource)
{
    stringbuilder s;

    s << "0x" << std::hex << platformSource << " ";

    switch (platformSource)
    {
    case 0: s << "(Default)"; break;
    case 1: s << "(Auto)"; break;
    case 2: s << "(Manual)"; break;
    case 3: s << "(Software)"; break;
    default: s << "(Unknown)"; break;
    }

    return s;
}


string f4531::desc_autoPlatform(unsigned short autoPlatform)
{
    stringbuilder s;

    s << "0x" << std::hex << autoPlatform << " ";

    switch (autoPlatform)
    {
    case 0xff: s << "(Undefined/Failed)"; break;
    }

    return s;
}


string f4531::desc_autoDescr(unsigned short autoDescr)
{
    stringbuilder s;

    s << "0x" << std::hex << autoDescr << " ";

    switch (autoDescr)
    {
    case 0xff: s << "(Undefined/Failed)"; break;
    }

    return s;
}


void f4531::onPlatformChangeEvent(short hostIndex, short platform, short  platformSrc)
{
    stringbuilder s;

    s << "onPlatformChangeEvent \n"
        << "   hostIndex = " << hostIndex << "\n"
        << "   platform = " << platform << "\n"
        << "   platformSrc = " << desc_platformSource(platformSrc) << "\n";

    if (m_log)
    {
        m_log->append(s);
    }

    if (m_eventsLog)
    {
        m_eventsLog->append(s);
    }
}


void f4531::getFeatureInfos()
{
    if (m_currentLog)
    {
        m_currentLog->clear();
    }
    _getFeatureInfos();
}


void f4531::_getFeatureInfos()
{
    stringbuilder log;

    if (feature4531->GetFeatureInfos(&m_capabilityMask, &m_numPlatforms, &m_numPlatformDescr, &m_numHosts, &m_currentHost))
    {
        log << "getFeatureInfos succeeded" << "\n"
            << "   capabilityMask = " << desc_capabilityMask(m_capabilityMask) << "\n"
            << "   numPlatforms = " << m_numPlatforms << "\n"
            << "   numPlatformDescr = " << m_numPlatformDescr << "\n"
            << "   numHosts = " << m_numHosts << "\n"
            << "   currentHost = " << m_currentHost << "\n"
            << "   currentHostPlatform = " << m_currentHostPlatform << "\n";
    }
    else
    {
        log << "getFeatureInfos failed \n" 
            << "   err = " << describe_err() << "\n";
    }

    log << "\n";

    if (m_log)
    {
        m_log->append(log);
    }

    if (m_currentLog)
    {
        m_currentLog->append(log);
    }
}


void f4531::getPlatformDescriptors()
{
    if (m_currentLog)
    {
        m_currentLog->clear();
    }
    _getPlatformDescriptors();
}


void f4531::_getPlatformDescriptors()
{    
    stringbuilder log;

    for (int i = 0; i < m_numPlatformDescr; i++)
    {
        std::vector<devio::Byte> platformDescriptor;
        if (feature4531->GetPlatformDescriptor(i, &platformDescriptor))
        {
            log << "getPlatformDescriptor succeeded, index = " << i << "\n"
                << "   Platform Index = 0x" << std::hex << (unsigned int)platformDescriptor[0] << "\n"
                << "   PlatformDescriptorIndex = 0x" << std::hex << (unsigned int)platformDescriptor[1] << "\n"
                << "   OS = 0x" << desc_platform(platformDescriptor[2].msb_word()) << "\n"
                << "   From Version = 0x" << std::hex << (unsigned int)platformDescriptor[4] << "\n"
                << "   From Revision = 0x" << std::hex << (unsigned int)platformDescriptor[5] << "\n"
                << "   To Version = 0x" << std::hex << (unsigned int)platformDescriptor[6] << "\n"
                << "   To Revision = 0x" << std::hex << (unsigned int)platformDescriptor[7] << "\n";
        }
        else
        {
            log << "getPlatformDescriptor failed, index = " << i << "\n"
                << "   err = " << describe_err() << "\n";
        }
        log << "\n";
    }

    log << "\n";

    if (m_log)
    {
        m_log->append(log);
    }

    if (m_currentLog)
    {
        m_currentLog->append(log);
    }
}


void f4531::getHostPlatforms()
{
    if (m_currentLog)
    {
        m_currentLog->clear();
    }
    _getHostPlatforms();
}


void f4531::setHostPlatform()
{
    stringbuilder log;

    int hostIndex = m_hostIndexSpinBox->value();
    int platform = m_platformSpinBox->value();

    bool ok = feature4531->setHostPlatform(hostIndex, platform);

    if (ok)
    {
        log << "setHostPlatform succeeded \n"
            << "   hostIndex = 0x" << std::hex << (unsigned int)hostIndex << "\n"
            << "   platform = " << std::dec << (unsigned int)platform << "\n";
    }
    else
    {
        log << "setHostPlatform failed \n"
            << "   hostIndex = 0x" << std::hex << (unsigned int)hostIndex << "\n"
            << "   platform = " << std::dec << (unsigned int)platform << "\n"
            << "   err = " << describe_err() << "\n";
    }


    if (m_log)
    {
        m_log->append(log);
    }

    if (m_currentLog)
    {
        m_currentLog->clear();
        m_currentLog->append(log);
    }
}


void f4531::_getHostPlatforms()
{
    stringbuilder log;

    // get the per channel , platform information
    for (int i = 0; i < m_numHosts; i++)
    {
        short hostIndex = i;

        short status = 0;
        short autoPlatform = 0;
        short hostIndexOut = 0;
        short platformSource = 0;
        short autoDescr = 0;
        short platform = 0;

        bool ok = feature4531->GetHostPlatform(hostIndex, &hostIndexOut, &status, &platform, &platformSource, &autoPlatform, &autoDescr);

        if (ok)
        {
            log << "getHostPlatform succeeded \n"
                << "   hostIndex = " << hostIndex << "\n"
                << "   status = " << desc_pairingStatus(status) << "\n"
                << "   platform = " << desc_platform(platform) << "\n"
                << "   platformSource = " << desc_platformSource(platformSource) << "\n"
                << "   autoPlatform = " << desc_autoPlatform(autoPlatform) << "\n"
                << "   autoDescr = " << desc_autoDescr(autoDescr) << "\n";
        }
        else
        {
            log << "getHostPlatform failed \n"
                << "   hostIndex = " << hostIndex << "\n"
                << "   err = " << describe_err() << "\n";
        }

        log << "\n";
    }

    if (m_log)
    {
        m_log->append(log);
    }

    if (m_currentLog)
    {
        m_currentLog->append(log);
    }
}


void f4531::logContextualMenu(const QPoint& point)
{
    (void)point;

    QMenu *menu = m_log->createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(QString("Clear"), this, SLOT(clearLog()));
    menu->exec(QCursor::pos());
}


void f4531::clearLog()
{
    m_log->clear();
}


void f4531::subscribeToEvents(bool enable)
{
    stringbuilder s;

    if (enable)
    {
        bool result = subscribe(feature4531, Delivery::Immediate);
        s << "Subscribing to feature 0x4531 reports \n"
          << "   result = " << describe_err(result) << "\n";
    }
    else
    {
        unsubscribe(feature4531);
        s << "Unsubscribing to feature 0x4531 reports \n";
    }
    
    if (m_log)
    {
        m_log->append(s);
    }    

    if (m_eventsLog)
    {
        m_eventsLog->append(s);
    }
}