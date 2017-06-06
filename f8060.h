#pragma once

#include <QTextEdit>
#include <QLineEdit>
#include "feature.h"
#include <QVBoxLayout>
#include <QComboBox>
#include <vector>
#include "devio.h"
#include "cInputTracker.h"

using devio::IDevice;
using devio::IFeature;
using devio::IFeature8060ReportRate;
using devio::Byte;
using devio::Subscriber;
using std::vector;

class f8060 : public QWidget
{
    Q_OBJECT
public:
    explicit f8060(shared_ptr<devio::IFeature8060ReportRate> f, Feature* base, shared_ptr<IDevice> = shared_ptr<IDevice>());
    
protected:
    void createUI(void);
    void refreshData(void);
    void refreshUI(void);
    void notifyText(const QString &text, bool insertCRLF = false);

protected slots:
    void onNotificationContextMenu(const QPoint &pos);
    void onClearNotifications(void);
    void onReportRateChanged(int index);
    void onGetReportRate(void);

#ifdef Q_OS_WIN
    private slots:
    void onRawInputDlg(void);
    void onInputTrackerDlgDestroyed(QObject*);
private:
    cInputTracker *m_inputTrackerDlg;

#endif
    
private:
    shared_ptr<devio::IFeature8060ReportRate> feature8060;
    Feature* baseFeature;
    weak_ptr<IDevice> const _device;

#ifdef Q_OS_WIN
#endif

    QTextEdit* notifications;
    
    uint8_t curReportRate;
    Byte reportRates;

    QComboBox* uiReportRates;
};
