#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include "feature.h"
#include "devio.h"
#include <vector>

using devio::IFeature;
using devio::IFeature8300Sidetone;
using devio::Byte;
using std::vector;


class f8300 : public QWidget
{
    Q_OBJECT

public:
    explicit f8300(shared_ptr<devio::IFeature8300Sidetone> f, Feature *base);

private slots:
    void onSetSidetoneLevel(void);
    void onGetSidetoneLevel(void);
    void onContextMenu(const QPoint& pt);
    void onClearNotifications(void);

private:
    void createUI(void);
    void notifyText(const QString &text, bool insertCRLF = false);

    shared_ptr<devio::IFeature8300Sidetone> feature8300;
    Feature *baseFeature;
    QTextEdit *notifications;

    // UI
    QPushButton* setSidetoneLevel;
    QPushButton* getSidetoneLevel;
    QLineEdit* levelEdit;
};
