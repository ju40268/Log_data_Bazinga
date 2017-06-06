#pragma once

#include <QTextEdit>
#include "feature.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QMenu>
#include <QGroupBox>
#include <QCheckBox>
#include <QTimer>
#include <QButtonGroup>
#include <QRadioButton>
#include <vector>
#include "devio.h"

using devio::IFeature;
using devio::IFeature8070ColorLEDEffects;
using devio::Byte;
using devio::Subscriber;
using std::vector;

class f8070 : public QWidget
{
    struct EffectUsageHint 
    {
        IFeature8070ColorLEDEffects::EffectId id;
        QString params[10];
    };

    struct NonVolatileSetting
    {
        IFeature8070ColorLEDEffects::NvCapability capability;
        QString name;
    };

    struct ExtendedCapabilities
    {
        IFeature8070ColorLEDEffects::ExtCapability capability;
        QString name;
    };

    Q_OBJECT
public:
    explicit f8070(shared_ptr<devio::IFeature8070ColorLEDEffects> f, Feature* base);
    void createUI(void);

    static QString toString(const IFeature8070ColorLEDEffects::ZoneLocation& loc);
    static QString toString(const IFeature8070ColorLEDEffects::EffectId& id);
    static QString toString(const IFeature8070ColorLEDEffects::EffectInfo& effectInfo);

private slots:
    void onZoneChanged(int index);
    void onZoneEffectChanged(int index);
    void onApplyEffect(void);
    void onGetCurrentColor(void);
    void onGetCurrentEffectSettings(void);
    void onColorTimerToggled(bool checked);
    void onSWControlToggled(bool checked);
    int featureVersion(void);
    void onGetZoneEffect(void);
    void onSetEffectOptionChanged(int option);
    void onContextMenu(const QPoint& pt);
    void onSyncDialog(void);
    void getSelectedNvConfig(void);
    void setSelectedNvConfig(void);

private:
    QColor gamma(const QColor& color);
    QColor inverseGamma(const QColor& color);

private:
    void dumpZoneEffect(int zoneIndex, int zoneEffectIndex, vector<Byte>& effectParams);

private:
    void createNonVolatileUI(QLayout* mainLayout);

private:
   QLineEdit* _params[10];
   QComboBox* _zoneList;
   QComboBox* _effectsList;
   QGridLayout* _paramsGrid;
   QLabel* _effectInfo;
   QCheckBox* _enableGammaCorrection;
   QCheckBox* _enableInverseGammaCorrection;
   QLabel* _colorPreview;
   QLabel* _rgbPreview;
   QTimer* _colorTimer;
   static const EffectUsageHint _usageHints[];
   QTextEdit *notifications;
   IFeature8070ColorLEDEffects::SetEffectOption m_setEffectOption; 
   IFeature8070ColorLEDEffects::NvCapability _nvCapabilities;
   IFeature8070ColorLEDEffects::ExtCapability _extCapabilities;
   QComboBox* _nvSettingsList;
   static const NonVolatileSetting _nonVolatileSettings[];
   static const ExtendedCapabilities _extendedCapabilities[];
   QLineEdit* _nvValue;
   QLineEdit* _nvParam1;
   QLineEdit* _nvParam2;

private:
    shared_ptr<devio::IFeature8070ColorLEDEffects> feature8070;
};
