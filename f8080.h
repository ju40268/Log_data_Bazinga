#pragma once

#include <QTextEdit>
#include "feature.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QTimer>
#include <vector>
#include "devio.h"
#include "Source/Feature8080PerKeyLighting_Interface.h"

using devio::IFeature;
using devio::IFeature8080PerKeyLighting;
using devio::Byte;
using devio::Subscriber;
using std::vector;

class f8080 : public QWidget
{
    Q_OBJECT
public:
    explicit f8080(shared_ptr<devio::IFeature8080PerKeyLighting> f, Feature* base);
    void createUI(void);
    void refreshData(void);
    void refreshUI(void);
private slots:
    void onUIChanged();
    void onZoneChanged();
    void onPersistenceToggled();
    void onSetOne();
    void onSetAll();
    void onSetRand();
    void onQueryColors();
    void onSetTimeRand();
    void onRandLoop();
    void spinboxValueChanged(int value);

private:
    void assignNames();
    void setOK(bool ok);
    QString toKeyTypeName(IFeature8080PerKeyLighting::KeyType);
    string toKeyboardLayoutName(uint16_t layoutID);

    IFeature8080PerKeyLighting::KeyType curZone();
    shared_ptr<devio::IFeature8080PerKeyLighting> m_f;
    vector<IFeature8080PerKeyLighting::KeyColorCombination> m_keyColors;
    QList<QString> keyNames;
    QList<QString> zoneNames;

    //RGB
    QSpinBox* m_colorR;
    QSpinBox* m_colorG;
    QSpinBox* m_colorB;
    QColor m_curColor;
    QLabel* m_colorIcon;

    // UpdateLEDs persistence option
    QCheckBox * m_persistence;
    IFeature8080PerKeyLighting::PersistenceOptions m_persistence_state;

    //Buttons
    QPushButton* m_setOne;
    QPushButton* m_setAll;
    QPushButton* m_setRand;
    QLabel* m_ok;

    //Zone selector
    QComboBox* m_zone;
    QListWidget* m_keys;

    IFeature8080PerKeyLighting::LightingInfo m_lightInfo;
    IFeature8080PerKeyLighting::KeyTypeInfo m_keyInfo;

    void assignKeyColor(QColor c, IFeature8080PerKeyLighting::KeyColorCombination &k);
    void assignQColor(IFeature8080PerKeyLighting::KeyColorCombination k, QColor &c);

    QLabel* m_keyboardMap;
    QPixmap m_colorOpts;
    QTimer m_randTimer;
};
