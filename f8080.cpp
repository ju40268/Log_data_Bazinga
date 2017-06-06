#include "feature.h"
#include "f8080.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include <QMenu>
#include <QGroupBox>
#include <QPainter>
#include "QSpinBox"
using devio::Delivery;

f8080::f8080(shared_ptr<devio::IFeature8080PerKeyLighting> f, Feature* base)
    :
    m_ok(0)
{
    m_f = f;
    refreshData();
    assignNames();
    createUI();
    refreshUI();
}


void f8080::createUI(void)
{
    QVBoxLayout* container = new QVBoxLayout(this);
    
    //Keyboard Layout
    {
        QString keyboardLayout;
        IFeature8080PerKeyLighting::LightingInfo info;
        info.keyboardLayout = 0;
        bool ok = m_f->GetInfo(info);
        keyboardLayout.append(stringbuilder()
            << "(KeyboardLayout = "
            << toKeyboardLayoutName(info.keyboardLayout)
            << ") -> "
            << describe_err(ok)
            );
        container->addWidget(new QLabel(keyboardLayout));
    }

    //RGB selector
    container->addWidget(new QLabel("Color (RGB)"));
    {
        QWidget* colorBar = new QWidget(this);
        QHBoxLayout* colors = new QHBoxLayout(colorBar);
        container->addWidget(colorBar);

        m_colorR = new QSpinBox( colorBar );
        m_colorR->setRange(0,255);
        m_colorR->setSingleStep(5);
        colors->addWidget(m_colorR);
        connect(m_colorR, SIGNAL(valueChanged (int)), this, SLOT(onUIChanged()));

        m_colorG = new QSpinBox( colorBar );
        m_colorG->setRange(0,255);
        m_colorG->setSingleStep(5);
        colors->addWidget(m_colorG);
        connect(m_colorG, SIGNAL(valueChanged (int)), this, SLOT(onUIChanged()));

        m_colorB = new QSpinBox( colorBar );
        m_colorB->setRange(0,255);
        m_colorB->setSingleStep(5);
        colors->addWidget(m_colorB);
        connect(m_colorB, SIGNAL(valueChanged (int)), this, SLOT(onUIChanged()));

        m_colorIcon = new QLabel(colorBar);
        colors->addWidget(m_colorIcon);
    }

    // UpdateLEDs persistence option
    {
        IFeature8080PerKeyLighting::LightingInfo info;
        info.keyboardLayout = 0;
        bool ok = m_f->GetInfo(info);
        if (ok && info.extFlags&IFeature8080PerKeyLighting::LightingInfoFlags::NON_VOLATILE_EFFECT) {
            QWidget* persistenceBar = new QWidget(this);
            QHBoxLayout* persistences = new QHBoxLayout(persistenceBar);
            container->addWidget(persistenceBar);

            m_persistence = new QCheckBox("Persistence");
            m_persistence->setChecked(false);
            connect(m_persistence, SIGNAL(toggled(bool)), this, SLOT(onPersistenceToggled()));
            persistences->addWidget(m_persistence);
        }

        m_persistence_state = IFeature8080PerKeyLighting::PersistenceOptions::PERSISTENCE_RAM;

    }


    //Push buttons
    {

        QWidget* buttonBar = new QWidget(this);
        QHBoxLayout* buttons = new QHBoxLayout(buttonBar);
        container->addWidget(buttonBar);

        m_zone = new QComboBox(buttonBar);

        // Go through all the possibilities
        for (int i = 0; i < 16; i++)
        {
            IFeature8080PerKeyLighting::KeyType kt = (IFeature8080PerKeyLighting::KeyType)(1 << i);
            if (m_lightInfo.supportedTypes & kt)
            {
                m_zone->addItem(toKeyTypeName(kt), kt);
            }
        }
        buttons->addWidget(m_zone);
        connect(m_zone, SIGNAL(currentIndexChanged(int)), this, SLOT(onZoneChanged()));

        m_setOne = new QPushButton("Set selected", buttonBar);
        buttons->addWidget(m_setOne);
        connect(m_setOne, SIGNAL(clicked()), this, SLOT( onSetOne() ));

        m_setAll = new QPushButton("Set All", buttonBar);
        buttons->addWidget(m_setAll);
        connect(m_setAll, SIGNAL(clicked()), this, SLOT( onSetAll() ));
        
        m_setRand = new QPushButton("Set All Random", buttonBar);
        buttons->addWidget(m_setRand);
        connect(m_setRand, SIGNAL(clicked()), this, SLOT(onSetRand()));

        QPushButton* cycleRand = new QPushButton("Simulate Effect", buttonBar);
        cycleRand->setToolTip("Sets random colors 32 times a second, to simulate software effects. Press again to turn off.");
        buttons->addWidget(cycleRand);
        connect(cycleRand, SIGNAL(clicked()), this, SLOT(onSetTimeRand()));
        connect(&m_randTimer, SIGNAL(timeout()), this, SLOT(onRandLoop()));
        m_randTimer.setInterval(5);

        QPushButton* refresh = new QPushButton("Get Colors");
        buttons->addWidget(refresh);
        connect(refresh, SIGNAL(clicked()), this, SLOT(onQueryColors()));

        m_ok = new QLabel("status: none");
        buttons->addWidget(m_ok);
    }
    {
        QWidget* softwareEffectWidget = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout();
        softwareEffectWidget->setLayout(layout);
        container->addWidget(softwareEffectWidget, 0, Qt::AlignLeft);
        layout->addWidget(new QLabel("Simulate Software Effect:"));
        QHBoxLayout* layout2 = new QHBoxLayout();
        layout->addLayout(layout2);

        layout2->addWidget(new QLabel("Refresh Milliseconds:"));

        QSpinBox* milliSpiner = new QSpinBox();
        milliSpiner->setMinimum(5);
        milliSpiner->setMaximum(60);
        milliSpiner->setValue(32);
        layout2->addWidget(milliSpiner);

        QPushButton* cycleRand = new QPushButton("Start Effect", softwareEffectWidget);
        cycleRand->setToolTip("Sets random colors 32 times a second, to simulate software effects. Press again to turn off.");
        layout2->addWidget(cycleRand);
        connect(cycleRand, SIGNAL(clicked()), this, SLOT(onSetTimeRand()));
        connect(&m_randTimer, SIGNAL(timeout()), this, SLOT(onRandLoop()));
        connect(milliSpiner, SIGNAL(valueChanged(int)), this, SLOT(spinboxValueChanged(int)));
        m_randTimer.setInterval(32);
    }

    //Selection lists
    {
        QWidget* comboBar = new QWidget(this);
        QHBoxLayout* combos = new QHBoxLayout(comboBar);
        container->addWidget(comboBar);

        m_keys = new QListWidget(comboBar);
        m_keys->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
        combos->addWidget(m_keys);  
        m_keyboardMap = new QLabel();
        combos->addWidget(m_keyboardMap);
    }


    onUIChanged();
    onZoneChanged();

    onQueryColors();
}


void f8080::refreshData(void)
{
    m_f->GetInfo( m_lightInfo );
}

void f8080::onZoneChanged()
{
    bool ok = m_f->GetKeyTypeInfo(curZone(), m_keyInfo);
    m_keys->clear();
    m_keyColors.clear();
    setOK(ok);
    if(!ok) return;

    m_f->GetKeyColors(curZone(), 0, m_persistence_state, m_keyColors );

    switch (curZone())
    {
        case IFeature8080PerKeyLighting::KT_HidKeyboard:
        {
            for (unsigned int i = 0; i < m_keyColors.size(); i++)
            {
                if (m_keyColors[i].key > 0 && m_keyColors[i].key < keyNames.length())
                {
                    QString label = QString("%1 (%2)").arg(keyNames[m_keyColors[i].key]).arg(m_keyColors[i].key);

                    QListWidgetItem*  newKey = new QListWidgetItem(label);
                    newKey->setData(Qt::UserRole, QVariant(m_keyColors[i].key));
                    m_keys->insertItem(m_keyColors[i].key, newKey );
                }
                else
                {
                    // we don't have a name for this key

                    QListWidgetItem*  newKey = new QListWidgetItem(QString("Key %1").arg(i));
                    newKey->setData(Qt::UserRole, QVariant(i));
                    m_keys->insertItem(i, newKey);
                }
            }
        }
            break;

        case IFeature8080PerKeyLighting::KT_GKeys:
        {
            QStringList l;
            for (int i = 1; i <= m_keyInfo.keyCount; i++)
            {

                QListWidgetItem*  newKey = new QListWidgetItem(QString("G%1").arg(i));
                newKey->setData(Qt::UserRole, QVariant((uint)i));
                m_keys->insertItem(i, newKey);
            }
        }
            break;

        case IFeature8080PerKeyLighting::KT_HidConsumer:
        {
            for (unsigned int i = 0; i < (int)m_keyColors.size(); i++)
            {
                QString label ="";
                switch (m_keyColors[i].key)
                {
                    case 0xB5:      // Next Track
                        label =  "Next Track";
                        break;
                    case 0xB6:      // Prev Track
                        label =  "Prev Track";
                        break;
                    case 0xB7:      // Stop
                        label =  "Stop";
                        break;
                    case 0xCD:      // Play
                        label =  "Play";
                        break;
                    case 0xE2:      // Mute
                        label = "Mute";
                        break;
                    case 0xE9:      // Volume Up
                        label = "Volume Up";
                        break;
                    case 0xEA:      // Volume Down
                        label = "Volume Down";
                        break;
                    default:
                        label =  QString("Usage 0x%1").arg(m_keyColors[i].key, 0, 16);
                        break;
                }
                QListWidgetItem*  newKey = new QListWidgetItem(label);
                newKey->setData(Qt::UserRole, QVariant(m_keyColors[i].key));
                m_keys->insertItem(i, newKey);
            }
        }
            break;

        case IFeature8080PerKeyLighting::KT_KeyboardOther:
        {
            for (unsigned int i = 0; i < (unsigned int)m_keyColors.size(); i++)
            {
                QString label = "";
                switch (m_keyColors[i].key)
                {
                    case 1:      // Backlight Toggle
                       label =  "Backlight Toggle";
                        break;
                    case 2:      // GameMode Switch
                       label =  "GameMode Switch";
                        break;
                    case 3:      // CapsLockLEDIndicator
                       label =  "CapsLock LED Indicator";
                        break;
                    case 4:      // ScrollLockLEDIndicator
                       label =  "ScrollLock LED Indicator";
                        break;
                    case 5:      // NumLockLEDIndicator
                       label =  "NumLock LED Indicator";
                        break;
                    default:
                       label =  QString("Unknown (%1)").arg(m_keyColors[i].key);
                        break;
                }
                QListWidgetItem*  newKey = new QListWidgetItem(label);
                newKey->setData(Qt::UserRole, QVariant(m_keyColors[i].key));
                m_keys->insertItem(i, newKey);
            }
        }
        break;

    default:
        {
            QStringList l;
            for(int i=1; i <= m_keyInfo.keyCount; i++)
            {
                QListWidgetItem*  newKey = new QListWidgetItem(QString("Key %1").arg(i));
                newKey->setData(Qt::UserRole, QVariant((uint)i));
                m_keys->insertItem(i, newKey);
            }
        }
        break;
    }
    onQueryColors();
}

void f8080::refreshUI(void)
{
}

void f8080::onUIChanged(void)
{
    QPixmap pix(32,32);
    m_curColor = QColor(m_colorR->value(), m_colorG->value(), m_colorB->value());
    pix.fill(m_curColor);
    m_colorIcon->setPixmap(pix);
}

void f8080::onPersistenceToggled(void)
{
    Qt::CheckState cs = m_persistence->checkState();
    switch (cs) {
    case Qt::CheckState::Checked:  m_persistence_state = IFeature8080PerKeyLighting::PersistenceOptions::PERSISTENCE_RAM_EEPROM; break;
    case Qt::CheckState::Unchecked: m_persistence_state = IFeature8080PerKeyLighting::PersistenceOptions::PERSISTENCE_RAM; break;
    default: m_persistence_state = IFeature8080PerKeyLighting::PersistenceOptions::PERSISTENCE_RAM; break;
    }
}

void f8080 ::onSetOne()
{
    vector<IFeature8080PerKeyLighting::KeyColorCombination> keys;
    IFeature8080PerKeyLighting::KeyColorCombination k;
    
    assignKeyColor(m_curColor, k);
    for (auto key : m_keys->selectedItems())
    {
        k.key = key->data(Qt::UserRole).toInt();
        keys.push_back(k);
    }

    bool ok = m_f->SetKeyColors( curZone() , keys );
    setOK(ok);
    m_f->FlushLEDS(m_persistence_state);
    onQueryColors();
}

void f8080 ::onSetAll()
{
    bool ok = m_f->SetAllKeysToColor( curZone() , m_curColor.red(), m_curColor.green(), m_curColor.blue() );
    m_f->FlushLEDS(m_persistence_state);
    setOK(ok);
    onQueryColors();
}
void f8080 ::onSetRand()
{
    for(size_t i =0; i < m_keyColors.size(); i++)
    {
        QColor newcolor = QColor(qrand() % 255,qrand() % 255,qrand() % 255 );
        assignKeyColor(newcolor,m_keyColors[i]);
    }
    bool ok = m_f->SetKeyColors( curZone() , m_keyColors);

    setOK(ok);
    m_f->FlushLEDS(m_persistence_state);
    onQueryColors();
}


void f8080::assignKeyColor(QColor c, IFeature8080PerKeyLighting::KeyColorCombination &k)
{
    k.keyR = c.red();
    k.keyG = c.green();
    k.keyB = c.blue();
}

void f8080::assignQColor(IFeature8080PerKeyLighting::KeyColorCombination k, QColor &c)
{
   c.setRed(  k.keyR ) ;
   c.setGreen( k.keyG ); 
   c.setBlue( k.keyB ); 
}


IFeature8080PerKeyLighting::KeyType f8080::curZone()
{
    IFeature8080PerKeyLighting::KeyType zone = (IFeature8080PerKeyLighting::KeyType ) m_zone->currentData().toInt();
    return (IFeature8080PerKeyLighting::KeyType )(zone);
}

void f8080::setOK(bool ok)
{
    if(m_ok  == 0) return;
    m_ok->setText( ok ? " ok" : "failed");
}


QString f8080::toKeyTypeName(IFeature8080PerKeyLighting::KeyType kt)
{
    switch(kt)
    {
        case IFeature8080PerKeyLighting::KT_HidKeyboard:
            return "KT_HidKeyboard";
        case IFeature8080PerKeyLighting::KT_HidConsumer:
            return "KT_HidConsumer";
        case IFeature8080PerKeyLighting::KT_GKeys:
            return "KT_GKeys";
        case IFeature8080PerKeyLighting::KT_GenericButton:
            return "KT_GenericButton";
        case IFeature8080PerKeyLighting::KT_Logo:
            return "KT_Logo";
        case IFeature8080PerKeyLighting::KT_Keyboard27Hz:
            return "KT_Keyboard27Hz";
        case IFeature8080PerKeyLighting::KT_KeyboardOther:
            return "KT_KeyboardOther";
        default:
            return QString("Unknown (0x%1)").arg((quint16)kt, 0, 16);
    }
}


string f8080::toKeyboardLayoutName(uint16_t layoutID)
{
    switch (layoutID)
    {
    case 1:
        return "US";
        break;
    case 2:
        return "International";
        break;
    case 3:
        return "UK";
        break;
    case 4:
        return "German";
        break;
    case 5:
        return "French";
        break;
    case 6:
        return "German/French";
        break;
    case 7:
        return "Russian";
        break;
    case 8:
        return "Pan Nordic";
        break;
    case 9:
        return "Korean";
        break;
    case 10:
        return "Japanese";
        break;
    case 11:
        return "Chinese Traditional";
        break;
    case 12:
        return "Chinese Simplified";
        break;
    default:
        break;
    }
    return "Undefined";
}

void f8080::assignNames()
{

    zoneNames.push_back("KT_HidKeyboard  ");
    zoneNames.push_back("KT_HidConsumer  ");
    zoneNames.push_back("KT_GKeys        ");
    zoneNames.push_back("KT_GenericButton");
    zoneNames.push_back("KT_Logo         ");
    zoneNames.push_back("KT_Keyboard27Hz ");
    zoneNames.push_back("KT_KeyboardOther ");

    /* i'm so sorry */
    keyNames.push_back( "-hidden-");
    keyNames.push_back( "-hidden-");
    keyNames.push_back( "-hidden-");
    keyNames.push_back( "-hidden-");
    keyNames.push_back( "A");
    keyNames.push_back( "B");
    keyNames.push_back( "C");
    keyNames.push_back( "D");
    keyNames.push_back( "E");
    keyNames.push_back( "F");
    keyNames.push_back( "G");
    keyNames.push_back( "H");
    keyNames.push_back( "I");
    keyNames.push_back( "J");
    keyNames.push_back( "K");
    keyNames.push_back( "L");
    keyNames.push_back( "M");
    keyNames.push_back( "N");
    keyNames.push_back( "O");
    keyNames.push_back( "P");
    keyNames.push_back( "Q");
    keyNames.push_back( "R");
    keyNames.push_back( "S");
    keyNames.push_back( "T");
    keyNames.push_back( "U");
    keyNames.push_back( "V");
    keyNames.push_back( "W");
    keyNames.push_back( "X");
    keyNames.push_back( "Y");
    keyNames.push_back( "Z");
    keyNames.push_back( "1");
    keyNames.push_back( "2");
    keyNames.push_back( "3");
    keyNames.push_back( "4");
    keyNames.push_back( "5");
    keyNames.push_back( "6");
    keyNames.push_back( "7");
    keyNames.push_back( "8");
    keyNames.push_back( "9");
    keyNames.push_back( "0");
    keyNames.push_back( "Return (ENTER)");
    keyNames.push_back( "ESCAPE");
    keyNames.push_back( "DELETE (Backspace)");
    keyNames.push_back( "Tab");
    keyNames.push_back( "Spacebar");
    keyNames.push_back( "-underscore");
    keyNames.push_back( "=");
    keyNames.push_back( "[");
    keyNames.push_back( "]");
    keyNames.push_back( "\\");
    keyNames.push_back( "Non-US #");
    keyNames.push_back( ";");
    keyNames.push_back( "'");
    keyNames.push_back( "~");
    keyNames.push_back( "");
    keyNames.push_back( ".");
    keyNames.push_back( "/");
    keyNames.push_back( "Caps Lock");
    keyNames.push_back( "F1");
    keyNames.push_back( "F2");
    keyNames.push_back( "F3");
    keyNames.push_back( "F4");
    keyNames.push_back( "F5");
    keyNames.push_back( "F6");
    keyNames.push_back( "F7");
    keyNames.push_back( "F8");
    keyNames.push_back( "F9");
    keyNames.push_back( "F10");
    keyNames.push_back( "F11");
    keyNames.push_back( "F12");
    keyNames.push_back( "PrintScreen");
    keyNames.push_back( "Scroll Lock");
    keyNames.push_back( "Pause");
    keyNames.push_back( "Insert");
    keyNames.push_back( "Home");
    keyNames.push_back( "PageUp");
    keyNames.push_back( "Delete Forward");
    keyNames.push_back( "End");
    keyNames.push_back( "PageDown");
    keyNames.push_back( "RightArrow");
    keyNames.push_back( "LeftArrow");
    keyNames.push_back( "DownArrow");
    keyNames.push_back( "UpArrow");
    keyNames.push_back( "Keypad Num Lock");
    keyNames.push_back( "Keypad /");
    keyNames.push_back( "Keypad *");
    keyNames.push_back( "Keypad -");
    keyNames.push_back( "Keypad +");
    keyNames.push_back( "Keypad ENTER");
    keyNames.push_back( "Keypad 1");
    keyNames.push_back( "Keypad 2");
    keyNames.push_back( "Keypad 3");
    keyNames.push_back( "Keypad 4");
    keyNames.push_back( "Keypad 5");
    keyNames.push_back( "Keypad 6");
    keyNames.push_back( "Keypad 7");
    keyNames.push_back( "Keypad 8");
    keyNames.push_back( "Keypad 9");
    keyNames.push_back( "Keypad 0");
    keyNames.push_back( "Keypad .");
}


void f8080::onQueryColors()
{
    m_f->GetKeyColors(curZone(), 0, m_persistence_state, m_keyColors);
    int swatchSize = 15;
    int canvasSize = 0x10 * swatchSize;
    m_colorOpts = QPixmap(canvasSize,canvasSize);
    m_colorOpts.fill(QColor(0, 0, 0, 0));
    
    QPainter* p = new QPainter(&m_colorOpts);
    p->setPen(QColor("gray"));
    p->setBrush(QColor("gray"));

    //Alpha lines
    for (int i = 0; i <canvasSize; i += swatchSize/2.41)
    {
        p->drawLine(0, i, canvasSize    , canvasSize + i);
        p->drawLine(i, 0, canvasSize + i, canvasSize    );
    }
    
    for (auto key : m_keyColors)
    {
        int y = (1 + ((key.key & 0xf0)>>4))*swatchSize;
        int x = (1 + (key.key & 0x0f))*swatchSize;
        QColor c = QColor(key.keyR, key.keyG, key.keyB);
        p->setPen(c);
        p->setBrush(c);

        p->drawRect(x, y, swatchSize, swatchSize);
    }

    p->setFont(QFont("Arial", swatchSize-2));
    for (int i = 0; i <= 0xf; i++)
    {
        QString label = QString::number(i, 16).toUpper();
        p->drawText((1 + i)*swatchSize, 0, swatchSize, swatchSize, Qt::AlignVCenter
            | Qt::AlignCenter, label);
        p->drawText(0, (1 + i)*swatchSize, swatchSize, swatchSize, Qt::AlignVCenter
            | Qt::AlignCenter, label);
    }
    
    p->end(); 

    m_keyboardMap->setPixmap(m_colorOpts);

}

//*************************************************************************
//
// f8080::onSetTimeRand
//
//*************************************************************************

void f8080::onSetTimeRand()
{
    if (m_randTimer.isActive())
    {
        ((QPushButton*)this->sender())->setText("Start Effect");
        m_randTimer.stop();
    }
    else
    {
        ((QPushButton*)this->sender())->setText("Stop Effect");
        m_randTimer.start();
    }
}

//*************************************************************************
//
// f8080::onRandLoop
//
//*************************************************************************

void f8080::onRandLoop()
{
    for (size_t i = 0; i < m_keyColors.size(); i++)
    {
        QColor newcolor = QColor(qrand() % 255, qrand() % 255, qrand() % 255);
        assignKeyColor(newcolor, m_keyColors[i]);
    }
    bool ok = m_f->SetKeyColors(curZone(), m_keyColors);

    m_f->FlushLEDS();
    int interval = m_randTimer.interval();
    if (interval)
    {
        interval = 0;
    }
}


//*************************************************************************
//
// f8080::spinboxValueChanged
//
//*************************************************************************

void f8080::spinboxValueChanged(int value)
{
    m_randTimer.setInterval(value);
}
