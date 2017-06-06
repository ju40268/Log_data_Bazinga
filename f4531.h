#pragma once

#include <QTextEdit>
#include "feature.h"
#include "devio.h"
#include <QTabWidget>
#include <memory>
#include <qspinbox.h>

using devio::IFeature;
using devio::IFeature4531MultiPlatform;
using devio::Subscriber;
using std::string;
using std::unique_ptr;

class f4531 : public QTabWidget,
	public Subscriber<devio::IFeature4531MultiPlatform::Report>
{
    Q_OBJECT
public:
	explicit f4531(shared_ptr<devio::IFeature4531MultiPlatform> f, Feature* base);

public:
// IFeature4531MultiPlatform::Report
	virtual void onPlatformChangeEvent(short hostIndex, short platform, short  platformSrc) override;

private slots:
    void getFeatureInfos();
    void getPlatformDescriptors();
    void getHostPlatforms();
    void setHostPlatform();
    void logContextualMenu(const QPoint& point);
    void clearLog();
    
private:
    void _getFeatureInfos();
    void _getPlatformDescriptors();
    void _getHostPlatforms();
    void subscribeToEvents(bool enable);

	shared_ptr<devio::IFeature4531MultiPlatform> feature4531;
    Feature* baseFeature;

    string desc_capabilityMask(unsigned short capabilityMask);
    string desc_pairingStatus(unsigned short status);
    string desc_platform(short platform);
    string desc_platformSource(unsigned short platformSource); 
    string desc_autoPlatform(unsigned short autoPlatform);
    string desc_autoDescr(unsigned short autoDescr);

	short  m_platform;
	int    m_capabilityMask;
	short  m_numPlatforms;
	short  m_numPlatformDescr;
	short  m_numHosts;
	short  m_currentHost;
    short  m_currentHostPlatform;

    unique_ptr<QWidget>     m_featureInfos;
    unique_ptr<QTextEdit>   m_log;
    unique_ptr<QTextEdit>   m_currentLog;
    unique_ptr<QTextEdit>   m_eventsLog;

    unique_ptr<QSpinBox>    m_hostIndexSpinBox;
    unique_ptr<QSpinBox>    m_platformSpinBox;
};
