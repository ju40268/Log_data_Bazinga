#pragma once

#include <QTextEdit>
#include "feature.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <vector>
#include "devio.h"

using devio::IFeature;
using devio::IFeature8110MouseButtonSpy;
using devio::Byte;
using devio::Subscriber;
using std::vector;

class f8110 : public QWidget,
              public Subscriber<devio::IFeature8110MouseButtonSpy::Report>
{
    Q_OBJECT
public:
    explicit f8110(shared_ptr<devio::IFeature8110MouseButtonSpy> f, Feature* base);
    
    virtual void onButtonReport(unsigned short newButtonState);

protected:
    void createUI(void);
    void notifyText(const QString &text, bool insertCRLF = false);
    void refreshData(void);
    void refreshUI(void);
    
protected slots:
    void onToggleSpy(void);
    void onButtonUsageChanged(int index);
    void onButtonMappedUsageChanged(int index);
    void onGetRemapping(void);
    void onResetButtonUsages(void);
    void dumpButtonUsages(void);
    void onNotificationContextMenu(const QPoint &pos);
    void onClearNotifications(void);
    
private:
    shared_ptr<devio::IFeature8110MouseButtonSpy> feature8110;
    Feature* baseFeature;
    QTextEdit* notifications;
    
    // data/hid++
    int buttonCount;
    vector<Byte> hidButtonMap;
    unsigned short buttonState;
    bool isSpying;
    
    // data/ui
    QLabel* uiButtonCount;
    QPushButton* uiSpyToggle;
    QComboBox* buttonUsages;
    QComboBox* buttonMappedUsages;
};
