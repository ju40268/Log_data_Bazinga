#pragma once

#include <QTextEdit>
#include "feature.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <vector>
#include "devio.h"

using devio::IFeature;
using devio::IFeature4522DKU;
using devio::Byte;
using std::vector;

class f4522 : public QWidget
{
    Q_OBJECT
public:
    explicit f4522(shared_ptr<devio::IFeature4522DKU> f, Feature* base);
    void createUI(void);
    void notifyText(const QString &text, bool insertCRLF = false);

private slots:
    void onDisableKeys(void);
    void onEnableKeys(void);
    void onEnableAllKeys(void);
    void onNotificationContextMenu(const QPoint &pos);
    void onClearNotifications(void);

private:
    QLineEdit* disableKeys[16];
    QLineEdit* enableKeys[16];
    QTextEdit* notifications;

private:
    shared_ptr<devio::IFeature4522DKU> feature4522;
};
