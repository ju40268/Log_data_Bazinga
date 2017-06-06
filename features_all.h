#pragma once

#include "feature.h"
#include <QPlainTextEdit>
#include <QPushButton>
#include <sstream>
#include <iomanip>
#include <map>
#include <string>
#include "stringbuilder.h"
#include "feature.h"

using devio::IDevice;
using devio::Byte;
using std::vector;
using devio::ErrorCode;
using devio::Subscriber;

class FeaturesAll : 
    public FeatureBase,
    public Subscriber<IDevice::Report>,
    public Subscriber<IDevice::WriteReport>,
    public Subscriber<IDevice::ConfigChange>
{
    Q_OBJECT
public:
    explicit FeaturesAll(Device *dev, QStackedLayout *sl, shared_ptr<devio::IDevice> d, QLabel* errlabel);
    void activate() override;
    void settings_lost();
	//added write file
	void FeaturesAll::writeFile(string s);

    // IDevice::Report
    void onReport(const vector<Byte>&, unsigned int sys, unsigned int count, IDevice::ReportType) override;

    // IDevice::WriteReport
    void onWriteReport(const vector<Byte>&, ErrorCode, unsigned int sys) override;

    void onConfigChange() override;

public slots:
    void onContextMenu(const QPoint &pos);
    void onClear(void);
    void slotRead(unsigned int timestamp, const vector<Byte>, unsigned int sys, unsigned int count, IDevice::ReportType);
    void slotWrite(unsigned int timestamp, const vector<Byte>, ErrorCode, unsigned int sys);
    //void doSend(void);
	void recordData(void);
	void doOpenLock(void);
    void doCloseLock(void);
    void doUnpair(void);
    void validateCommandToSend();
private:
    shared_ptr<devio::IDevice> device;
    QPlainTextEdit *te;
    QPlainTextEdit *user_command;
    QPushButton *send_button;
	QPushButton *openlock_button;
    QPushButton *closelock_button;
    QPushButton *unpair_button;
	
public:
    static string timestamp(unsigned int int_timestamp);
    static string timestamp_full(unsigned int int_timestamp);
    static unsigned int int_timestamp();

signals:
    void signalRead(unsigned int timestamp, const vector<Byte>, unsigned int sys, unsigned int count, IDevice::ReportType);
    void signalWrite(unsigned int timestamp, const vector<Byte>, ErrorCode, unsigned int sys);

    
};
