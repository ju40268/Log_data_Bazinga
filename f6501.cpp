#include "feature.h"
#include "f6501.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QString>
#include <QHeaderView>
#include <QList>
#include <QMenu>

using devio::Delivery;
using devio::Byte;
using std::vector;

void f6501::slotClear()
{
    log->clear();
}

void f6501::contextualMenu(const QPoint& point)
{
    (void) point;

    QMenu *menu = new QMenu;

    menu->addAction(QString("Clear"), this, SLOT(slotClear()));
    menu->exec(QCursor::pos());
}

f6501::f6501(shared_ptr<devio::IFeature6501Gestures> feature6501_, Feature* base) :
    QTabWidget(),
    feature6501(feature6501_),
    baseFeature(base)
{
    // pages:
    // 1) text log
    // 2) descriptor (tree style gestures with children: tasks, actions, specs, params)
    // 3) enabled (multi-selection list)
    // 4) diverted (multi-selection list)
    // 5) specs (list, with data in columns)
    // 6) params (spin wheel to select index, auto-updating param id, buttons for default/get/set, editable text area to display hex data)

    // log page (textedit)
    log = new QTextEdit;
    log->setPlainText(baseFeature->description);
    log->setReadOnly(true);
    log->setWordWrapMode(QTextOption::NoWrap);

    log->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(log,SIGNAL(customContextMenuRequested(const QPoint &)),this,SLOT(contextualMenu(const QPoint &)));

    // raw descriptor page
    page_descriptor = new QTreeWidget;

    // gestures page (tree style gestures with children: tasks, actions, specs, params)
    page_gesture = new QTreeWidget;

    // enabled page (multi-selection list)
    page_enable = new QListWidget;

    // diverted page (multi-selection list)
    page_divert = new QListWidget;

    // specs page (list, with data in columns)
    page_spec = new QTreeWidget;

    // params page (spin wheel to select index, auto-updating param id, buttons for default/get/set, editable text area to display hex data)
    page_param = new QWidget;

    // read descriptor
    gestures.clear();
    for (unsigned int i=0; true; i++)
    {
        IFeature6501Gestures::GestureInfo info;
        bool ok = feature6501->GetGestureInfo( i, &info);
        log->append( stringbuilder()
                << "GetGestureInfo("
                << " index=" << i
                << ", gesture_id=" << (int)info.gesture_id
                << ", can_enable=" << info.can_enable
                << ", enable_default=" << info.enable_default
                << ", can_divert=" << info.can_divert
                << ", show_in_ui=" << info.show_in_ui
                << ", ui_default=" << info.ui_default
                << ", persistent=" << info.persistent
                << ", specs=" << info.specs.size()
                << ", params=" << info.params.size()
                << ", task_action=" << info.task_action.size()
                << ", report_type=" << (unsigned int) info.report_type
                << ") -> " << describe_err(ok)
                );
        if (!ok)
        {
            break;
        }

        gestures.push_back(info);
    }

    //
    // update page_descriptor
    page_descriptor->setSelectionMode(QAbstractItemView::NoSelection);
    page_descriptor->setColumnCount(3);
    page_descriptor->setHeaderLabels(QList<QString>() << QString("Type")  << QString("Subtype") << QString("Id"));
    page_descriptor->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    unsigned int descriptor_item = 0;
    unsigned int extendid = 0;
    while (true)
    {
        unsigned int val;
        bool ok = feature6501->GetRawDescriptor( descriptor_item, &val);
        log->append( stringbuilder()
                << "GetRawDescriptor("
                << " index=" << descriptor_item
                << ", val=0x" << std::hex << std::setfill('0') << std::setw(4) << val
                << ") -> " << describe_err(ok)
                );
        if (!ok)
        {
            break;
        }

        stringbuilder txt1;
        stringbuilder txt2;
        stringbuilder txt3;
        unsigned int val1;
        unsigned int val2;
        unsigned int val3;
        auto item = new QTreeWidgetItem();
        val1 = val >> 12;
        switch (val1)
        {
        case 0:
            txt1 << "SYNTAX";
            val2 = (val >> 8) & 0xF;
            switch (val2)
            {
            case 0: txt2 << "CONTINUE"; break;
            case 1: txt2 << "END"; break;
            case 2: txt2 << "EXTEND_ID";
                extendid = (extendid << 8) | (val & 0xFF);
                break;
            case 3: txt2 << "EXTEND_CAP"; break;
            case 4: txt2 << "GROUP";
                val3 = val & 0xFF;
                switch (val3)
                {
                case 0:  txt3 << "APPLY_TO_FOLLOWING"; break;
                case 1:  txt3 << "GLOBAL_CLEAR"; break;
                case 2:  txt3 << "GROUP_BEGIN"; break;
                case 3:  txt3 << "GROUP_END"; break;
                default: txt3 << "0x" << std::hex << val3; break;
                }
                break;
            default: txt2 << "0x" << std::hex << val2; break;
            }
            break;
        case 1:
            txt1 << "0x01";
            break;
        case 2:
        case 3:
            txt1 << "PARAM UI=" << ((val1 == 3) ? "yes" : "no");
            val2 = (val >> 8) & 0xF;
            txt2 << "LEN=" << val2;
            val3 = (extendid << 8) | (val & 0xFF);
            extendid = 0;
            txt3 << paramName((IFeature6501Gestures::ParamId)val3);
            break;
        case 4:
            txt1 << "SPEC";
            val2 = (val >> 8) & 0xF;
            txt2 << "LEN=" << val2;
            val3 = (extendid << 8) | (val & 0xFF);
            extendid = 0;
            txt3 << specName((IFeature6501Gestures::SpecId)val3);
            break;
        case 5:
            txt1 << "0x05";
            break;
        case 6:
            txt1 << "TASK";
            extendid = 0;
            break;
        case 7:
            txt1 << "ACTION";
            extendid = 0;
            break;
        default:
            txt1 << "GESTURE";
            val2 = (val >> 8) & 0x7F;
            txt2 << "0x" << std::hex << std::setw(2) << std::setfill('0') << val2;
            val3 = (extendid << 8) | (val & 0xFF);
            extendid = 0;
            txt3 << gestureName((IFeature6501Gestures::GestureId)val3);
            break;
        }
        item->setText(0,txt1);
        item->setText(1,txt2);
        item->setText(2,txt3);
        page_descriptor->addTopLevelItem(item);

        descriptor_item++;
    }

    // updage page_gesture
    page_gesture->setSelectionMode(QAbstractItemView::NoSelection);
    page_gesture->setColumnCount(5);
    page_gesture->setHeaderLabels(QList<QString>() << QString("Gesture") << QString("Enable") << QString("Divert") << QString("Ui") << QString("Persist"));
    page_gesture->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    for (unsigned int i=0; i < gestures.size(); i++)
    {
        IFeature6501Gestures::GestureInfo g = gestures[i];

        auto gesture_item = new QTreeWidgetItem();
        gesture_item->setText(0,stringbuilder() << gestureName(g.gesture_id));
        if (g.can_enable) gesture_item->setText(1,g.enable_default ? "E+" : "E-");
        if (g.can_divert) gesture_item->setText(2,"V");
        if (g.show_in_ui) gesture_item->setText(3,g.ui_default ? "U+" : "U-");
        if (g.persistent) gesture_item->setText(4,"P");

        for (unsigned int t=0; t < g.task_action.size(); t++)
        {
            auto item = new QTreeWidgetItem(gesture_item);
            if (g.task_action[t].is_task)
            {
                item->setText(0,stringbuilder() << "Task" << g.task_action[t].id);
            }
            else if (g.task_action[t].is_action)
            {
                item->setText(0,stringbuilder() << "Action" << g.task_action[t].id);
            }
            else
            {
                item->setText(0,stringbuilder() << "TaskActionOther" << g.task_action[t].id);
            }
        }

        for (unsigned int s=0; s < g.specs.size(); s++)
        {
            auto item = new QTreeWidgetItem(gesture_item);
            item->setText(0,stringbuilder() << "Spec index=" << g.specs[s].spec_index << " name=" << specName(g.specs[s].spec_id));
        }

        for (unsigned int p=0; p < g.params.size(); p++)
        {
            auto item = new QTreeWidgetItem(gesture_item);
            item->setText(0,stringbuilder() << "Param index=" << g.params[p].param_index << " name=" << paramName(g.params[p].param_id));
            item->setText(3,g.params[p].show_in_ui ? "yes" : "no");
            gesture_item->addChild(item);
        }

        page_gesture->addTopLevelItem(gesture_item);
        //page_gesture->expandItem(gesture_item);
    }

    // update page_enable
    page_enable->setSelectionMode(QAbstractItemView::NoSelection);
    for (unsigned int i=0; i < gestures.size(); i++)
    {
        IFeature6501Gestures::GestureInfo g = gestures[i];
        if (g.can_enable)
        {
            auto item = new QListWidgetItem(stringbuilder() << gestureName(g.gesture_id));

            bool enabled;
            bool ok = feature6501->IsEnabled(i, &enabled);
            log->append( stringbuilder()
                    << "IsEnabled("
                    << " index=" << i
                    << ", enabled=" << enabled
                    << ") -> " << describe_err(ok)
                    );

            page_enable->addItem(item);
            if (ok)
            {
                item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
            }
            else
            {
                item->setForeground(Qt::gray);
            }

            enable_list.push_back(i);
        }
    }

    // ui changing of enabled settings
    if (!connect(page_enable, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onEnableChanged(QListWidgetItem*))))
    {
        log->append("Error: Signal itemChanged to slot onEnableChanged not connected!");
    }

    // monitoring of an external app changing diverted gestures
    if (!connect(this, SIGNAL(signalOnGestureEnabled(unsigned int, unsigned int, bool)),
                 this, SLOT(slotOnGestureEnabled(unsigned int, unsigned int, bool)),
                 Qt::QueuedConnection))
    {
        log->append("Error: signalOnGestureEnabled to slotOnGestureEnabled not connected!");
    }

    // update page_divert
    page_divert->setSelectionMode(QAbstractItemView::NoSelection);
    for (unsigned int i=0; i < gestures.size(); i++)
    {
        IFeature6501Gestures::GestureInfo g = gestures[i];
        if (g.can_divert)
        {
            auto item = new QListWidgetItem(stringbuilder() << gestureName(g.gesture_id));

            bool diverted;
            bool ok = feature6501->IsDiverted(i, &diverted);
            log->append( stringbuilder()
                    << "IsDiverted("
                    << " index=" << i
                    << ", diverted=" << diverted
                    << ") -> " << describe_err(ok)
                    );

            if (ok)
            {
                item->setCheckState(diverted ? Qt::Checked : Qt::Unchecked);
            }
            else
            {
                item->setForeground(Qt::gray);
            }
            page_divert->addItem(item);
            divert_list.push_back(i);
        }
    }

    if (!connect(page_divert, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onDivertChanged(QListWidgetItem*))))
    {
        log->append("Error: Signal itemChanged to slot onDivertChanged  not connected!");
    }

    // monitoring of an external app changing diverted gestures
    if (!connect(this, SIGNAL(signalOnGestureDiverted(unsigned int, unsigned int, bool)),
                 this, SLOT(slotOnGestureDiverted(unsigned int, unsigned int, bool)),
                 Qt::QueuedConnection))
    {
        log->append("Error: signalOnGestureDiverted to slotOnGestureDiverted not connected!");
    }

    // update page_spec
    page_spec->setSelectionMode(QAbstractItemView::NoSelection);
    page_spec->setColumnCount(1);
    page_spec->setHeaderLabels(QList<QString>() << QString("Specs"));
    page_spec->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    unsigned int spec_count = 0;
    for (auto g : gestures)
    {
        for (auto s : g.specs)
        {
            if (s.spec_index >= spec_count)
            {
                spec_count = s.spec_index + 1;
            }
        }
    }

    for (unsigned int i=0; i < spec_count; i++)
    {
        vector<Byte> value;
        bool ok = feature6501->GetSpec(i, &value);
        stringbuilder str;
        str        << "GetSpec(";
        str        << " spec_index=" << i;
        str        << ", length=" << value.size();
        for (auto v : value)
        {
            str        << ", data=0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
        }
        str        << ") -> " << describe_err(ok);
        log->append( str);

        auto item = new QTreeWidgetItem();

        IFeature6501Gestures::SpecId found_spec_id = IFeature6501Gestures::SpecId::None;
        for (auto g : gestures)
        {
            IFeature6501Gestures::SpecId spec_id = IFeature6501Gestures::SpecId::None;

            for (auto s : g.specs)
            {
                if (s.spec_id == spec_id)
                {
                    spec_id = IFeature6501Gestures::SpecId::None;  // the found spec was overridden, so it doesn't apply
                }

                if (s.spec_index == i)
                {
                    spec_id = s.spec_id;
                }
            }

            if (spec_id != IFeature6501Gestures::SpecId::None)
            {
                auto gesture_item = new QTreeWidgetItem(item);
                gesture_item->setText(0,stringbuilder() << "Applies to " << gestureName(g.gesture_id));
                found_spec_id = spec_id;
            }
        }

        stringbuilder txt;
        txt << "index=" << i << ": " << specName(found_spec_id);
        txt << " = (hex)";
        for (auto v : value)
        {
            txt << " " << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
        }
        item->setText(0,txt);

        page_spec->addTopLevelItem(item);
    }

    // update page_param
    page_param_list = new QTreeWidget();
    page_param_text = new QLineEdit();
    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addWidget(page_param_get = new QPushButton("Get Value"));
    buttons->addWidget(page_param_set = new QPushButton("Set Value"));
    buttons->addWidget(page_param_def = new QPushButton("Get Default"));
    buttons->addWidget(page_param_text);
    {
        QVBoxLayout *v = new QVBoxLayout;
        v->addWidget(page_param_list);
        v->addLayout(buttons);
        //v->addWidget(page_param_text);
        //v->addStretch(1);
        page_param->setLayout(v);
    }

    page_param_list->setSelectionMode(QAbstractItemView::SingleSelection);
    page_param_list->setColumnCount(1);
    page_param_list->setHeaderLabels(QList<QString>() << QString("Params"));
    page_param_list->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    unsigned int param_count = 0;
    for (auto g : gestures)
    {
        for (auto p : g.params)
        {
            if (p.param_index >= param_count)
            {
                param_count = p.param_index + 1;
            }
        }
    }

    for (unsigned int i=0; i < param_count; i++)
    {
        vector<Byte> defvalue;

        {
            bool ok = feature6501->ParamDefault(i, &defvalue);
            stringbuilder str;
            str        << "ParamDefault(";
            str        << " param_index=" << i;
            str        << ", length=" << defvalue.size();
            for (auto v : defvalue)
            {
                str        << ", data=0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
            }
            str        << ") -> " << describe_err(ok);
            log->append( str);
        }

        param_lengths[i] = (unsigned int)defvalue.size();
        param_defaults[i] = defvalue;

        vector<Byte> value;

        {
            bool ok = feature6501->GetParam(i, &value);
            stringbuilder str;
            str        << "GetParam(";
            str        << " param_index=" << i;
            str        << ", length=" << value.size();
            for (auto v : value)
            {
                str        << ", data=0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
            }
            str        << ") -> " << describe_err(ok);
            log->append( str);
        }

        auto item = new QTreeWidgetItem();

        for (auto g : gestures)
        {
            IFeature6501Gestures::ParamId param_id = IFeature6501Gestures::ParamId::None;
            for (auto p : g.params)
            {
                if (p.param_id == param_id)
                {
                    param_id = IFeature6501Gestures::ParamId::None;  // the found param was overridden, so it doesn't apply
                }

                if (p.param_index == i)
                {
                    param_id = p.param_id;
                }
            }

            if (param_id != IFeature6501Gestures::ParamId::None)
            {
                auto gesture_item = new QTreeWidgetItem(item);
                gesture_item->setText(0,stringbuilder() << "Applies to " << gestureName(g.gesture_id));
            }
        }

        page_param_list->addTopLevelItem(item);
        setParamText( i, value);
    }

    if (!connect(page_param_list, SIGNAL(itemSelectionChanged()), this, SLOT(onParamSelected())))
    {
        log->append("Error: Signal itemSelectionChanged to slot onParamSelected not connected!");
    }

    if (!connect(page_param_text, SIGNAL(textChanged(const QString &)), this, SLOT(onParamTextChanged(const QString &))))
    {
        log->append("Error: Signal textChanged to slot onParamTextChanged not connected!");
    }

    if (!connect(page_param_get, SIGNAL(clicked()), this, SLOT(onClickGet())))
    {
        log->append("Error: Signal clicked to slot onClickGet not connected!");
    }

    if (!connect(page_param_set, SIGNAL(clicked()), this, SLOT(onClickSet())))
    {
        log->append("Error: Signal clicked to slot onClickSet not connected!");
    }

    if (!connect(page_param_def, SIGNAL(clicked()), this, SLOT(onClickDef())))
    {
        log->append("Error: Signal clicked to slot onClickDef not connected!");
    }

    updateButtons();

    // get the current mode so we can pre-select the correct mode control
    //devio::Byte oldmode = 0;
    //bool bresult = feature->GetRawReportState(&oldmode);
    //text->append(stringbuilder()
    //    << "GetRawReportState got mode 0x" << std::hex << (unsigned int)oldmode << std::dec << " -> "
    //    << describe_err(bresult)
    //    );

    //QGroupBox *groupBox = new QGroupBox(tr("Feature 6100 mode"));
    //QVBoxLayout *vbox = new QVBoxLayout;
    //for (int i=0; i < nummodes; i++)
   // {
   //     rbuttons[i] = new QRadioButton(QString("Mode 0x%1").arg(modes[i],0,16));
   //     rbuttons[i]->setChecked(oldmode == modes[i]);
   //     vbox->addWidget(rbuttons[i]);
   //     //rbuttons[i]->setAutoExclusive(false);
   //     QObject::connect(rbuttons[i],SIGNAL(clicked(bool)),this,SLOT(rbutton_clicked(bool)));
   // }

    //vbox->addStretch(1);
    //groupBox->setLayout(vbox);
    //QVBoxLayout *v = new QVBoxLayout;
    //v->addWidget(groupBox);
    //v->addStretch(1);
    //controls->setLayout(v);

    // add tabs to tab widget
    addTab(page_gesture,"Gestures");
    addTab(page_enable,"Enable");
    addTab(page_divert,"Divert");
    addTab(page_spec,"Specs");
    addTab(page_param,"Params");
    addTab(page_descriptor,"Descriptor");
    addTab(log,"Log");

    // gesture progress reports
    if (!connect(this, SIGNAL(signalGestureReport(QString)),
                 this, SLOT(slotGestureReport(QString)),
                 Qt::QueuedConnection))
    {
        log->append("Error: signalGestureReport to slotGestureReport not connected!");
    }

    bool bresult = subscribe(feature6501,Delivery::Immediate);
    log->append(stringbuilder()
        << "Subscribing to 6501 reports -> "
        << describe_err(bresult)
        );
}

void f6501::onEnableChanged(QListWidgetItem*item)
{
    unsigned int r = page_enable->row(item);
    log->append(stringbuilder() << "enable change row is "<<r);

    if (enable_list.size() <= r)
    {
        log->append(stringbuilder() << "enable change row does not exist");
        return;
    }

    unsigned int gesture_index = enable_list[r];

    //TODO: create a thread to do this
    bool enable = page_enable->item(r)->checkState() == Qt::Checked;
    bool ok = feature6501->Enable(gesture_index, enable);
    log->append( stringbuilder()
            << "Enable("
            << " gesture_index=" << gesture_index
            << ", enable=" << enable
            << ") -> " << describe_err(ok)
            );

    readEnabledState(gesture_index, item);
}

void f6501::onParamSelected()
{
    page_param_text->clear();
    updateButtons();
}

void f6501::onParamTextChanged(const QString &str)
{
    (void)str;
    updateButtons();
}

void f6501::setParamText( int param_index, vector<Byte> value)
{
    IFeature6501Gestures::ParamId param_id = param_ids[param_index];

    stringbuilder txt;
    txt << "index=" << param_index << ": " << paramName(param_id);
    txt << ", value = (hex)";
    for (auto v : value)
    {
        txt << " " << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
    }
    txt << ", default = (hex)";
    for (auto v : param_defaults[param_index])
    {
        txt << " " << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
    }

    auto item = page_param_list->topLevelItem(param_index);
    item->setText(0,txt);
}

vector<Byte> parse_hex( QString str, unsigned int buflen)
{
    vector<Byte> result;

    QString delimiterPattern(" ");
    QStringList words = str.split(" ");

    for (int i = 0; i < words.size(); i++)
    {
        QString word = words.at(i);

        if (word.length() > 2 && (word.length() & 1) == 1) return vector<Byte>();

        for (int j = 0; j < word.length(); j += 2)
        {
            bool ok;
            QString hexbyte = word.mid(j,2);
            int val = hexbyte.toInt( &ok, 16);

            if (!ok) return vector<Byte>();

            result.push_back(val);
        }
    }

    if (result.size() > buflen) return vector<Byte>();

    return result;
}

void f6501::updateButtons()
{
    int row = page_param_list->indexOfTopLevelItem ( page_param_list->currentItem());

    log->append( stringbuilder() << "row: " << row);

    bool get = false;
    bool set = false;
    bool def = false;

    if (row >= 0) get = true;
    if (row >= 0) def = true;
    if (row >= 0 && parse_hex(page_param_text->text(), param_lengths[row]).size()) set = true;

    page_param_get->setEnabled(get);
    page_param_set->setEnabled(set);
    page_param_def->setEnabled(def);
}

void f6501::onClickGet()
{
    int row = page_param_list->indexOfTopLevelItem ( page_param_list->currentItem());

    if (row >= 0)
    {
        vector<Byte> value;

        {
            bool ok = feature6501->GetParam(row, &value);
            stringbuilder str;
            str        << "GetParam(";
            str        << " param_index=" << row;
            str        << ", length=" << value.size();
            for (auto v : value)
            {
                str        << ", data=0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
            }
            str        << ") -> " << describe_err(ok);
            log->append( str);
        }

        stringbuilder txt;
        const char *spacing = "";
        for (auto v : value)
        {
            txt << spacing << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
            spacing = " ";
        }

        page_param_text->setText(txt);
    }
}

void f6501::onClickSet()
{
    int row = page_param_list->indexOfTopLevelItem ( page_param_list->currentItem());

    if (row >= 0)
    {
        vector<Byte> value = parse_hex(page_param_text->text(), param_lengths[row]);

        if (value.size())
        {
            value.resize(param_lengths[row]);

            // show the value properly formatted
            stringbuilder txt;
            const char *spacing = "";
            for (auto v : value)
            {
                txt << spacing << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
                spacing = " ";
            }
            page_param_text->setText(txt);

            bool ok = feature6501->SetParam(row, value);
            stringbuilder str;
            str        << "SetParam(";
            str        << " param_index=" << row;
            str        << ", length=" << value.size();
            for (auto v : value)
            {
                str        << ", data=0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
            }
            str        << ") -> " << describe_err(ok);
            log->append( str);

            setParamText( row, value);
        }
    }
}

void f6501::onClickDef()
{
    int row = page_param_list->indexOfTopLevelItem ( page_param_list->currentItem());

    if (row >= 0)
    {
        vector<Byte> value;

        {
            bool ok = feature6501->ParamDefault(row, &value);
            stringbuilder str;
            str        << "ParamDefault(";
            str        << " param_index=" << row;
            str        << ", length=" << value.size();
            for (auto v : value)
            {
                str        << ", data=0x" << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
            }
            str        << ") -> " << describe_err(ok);
            log->append( str);
        }

        stringbuilder txt;
        const char *spacing = "";
        for (auto v : value)
        {
            txt << spacing << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)v;
            spacing = " ";
        }

        page_param_text->setText(txt);
    }
}

void f6501::readEnabledState(unsigned int gesture_index, QListWidgetItem*item)
{
    bool enabled;
    bool ok = feature6501->IsEnabled(gesture_index, &enabled);
    log->append( stringbuilder()
            << "IsEnabled("
            << " gesture_index=" << gesture_index
            << ", enabled=" << enabled
            << ") -> " << describe_err(ok)
            );

    page_enable->blockSignals(true);
    item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
    page_enable->blockSignals(false);
}

void f6501::onDivertChanged(QListWidgetItem*item)
{
    unsigned int r = page_divert->row(item);
    log->append(stringbuilder() << "divert change row is "<<r);

    if (divert_list.size() <= r)
    {
        log->append(stringbuilder() << "divert change row does not exist");
        return;
    }

    unsigned int gesture_index = divert_list[r];

    //TODO: create a thread to do this
    bool divert = page_divert->item(r)->checkState() == Qt::Checked;
    bool ok = feature6501->Divert(gesture_index, divert);
    log->append( stringbuilder()
            << "Divert("
            << " divert_index=" << gesture_index
            << ", divert=" << divert
            << ") -> " << describe_err(ok)
            );

    readDivertedState( gesture_index, item);
}

void f6501::readDivertedState(unsigned int gesture_index, QListWidgetItem*item)
{
    bool diverted;
    bool ok = feature6501->IsDiverted(gesture_index, &diverted);
    log->append( stringbuilder()
            << "IsDiverted("
            << " divert_index=" << gesture_index
            << ", diverted=" << diverted
            << ") -> " << describe_err(ok)
            );

    page_divert->blockSignals(true);
    item->setCheckState(diverted ? Qt::Checked : Qt::Unchecked);
    page_divert->blockSignals(false);
}

void f6501::slotOnGestureEnabled(unsigned int swid, unsigned int gi, bool enabled)
{
    (void)swid;

    log->append(stringbuilder() << "external enable change row is " << gi);
    for (unsigned int i=0; i < enable_list.size(); i++)
    {
        if (gi == enable_list[i])
        {
            QListWidgetItem *item = page_enable->item(i);
            page_enable->blockSignals(true);
            item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
            page_enable->blockSignals(false);
        }
    }
}

void f6501::slotOnGestureDiverted(unsigned int swid, unsigned int gi, bool diverted)
{
    (void)swid;

    log->append(stringbuilder() << "external divert change row is " << gi);
    for (unsigned int i=0; i < divert_list.size(); i++)
    {
        if (gi == divert_list[i])
        {
            QListWidgetItem *item = page_divert->item(i);
            page_divert->blockSignals(true);
            item->setCheckState(diverted ? Qt::Checked : Qt::Unchecked);
            page_divert->blockSignals(false);
        }
    }
}

void f6501::OnGestureEnabled(unsigned int swid, unsigned int gi, bool enabled)
{
    emit signalOnGestureEnabled(swid, gi, enabled);
}

void f6501::OnGestureDiverted(unsigned int swid, unsigned int gi, bool diverted)
{
    emit signalOnGestureDiverted(swid, gi, diverted);
}

void f6501::OnParamChange(unsigned int swid, unsigned int pi, vector<Byte> new_value)
{
    (void)swid;
    (void)pi;
    (void)new_value;
}

void f6501::settings_lost()
{
    log->append("settings lost");

    for (unsigned int i=0; i < enable_list.size(); i++)
    {
        readEnabledState( enable_list[i], page_enable->item(i));
    }

    for (unsigned int i=0; i < divert_list.size(); i++)
    {
        readDivertedState( divert_list[i], page_divert->item(i));
    }
}

void f6501::slotGestureReport(QString txt)
{
    log->append(txt);
}

void f6501::OnGesture(unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type)
{
    unsigned int ts = int_timestamp();

    QString txt = stringbuilder()
        << timestamp(ts)
        << " - OnGesture("
        << " index=" << gi
        << ", id=" << (unsigned int)id
        << ", event_type=" << (unsigned int)event_type
        << ")";

    // We are not in the gui thread.. send report to gui thread.
    emit signalGestureReport(txt);
}

void f6501::OnGestureTriggerXY(unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type, int x, int y)
{
    unsigned int ts = int_timestamp();

    // We are not in the gui thread.. send report to gui thread.
    QString txt = stringbuilder()
        << timestamp(ts)
        << " - OnGestureTriggerXY("
        << " index=" << gi
        << ", id=" << (unsigned int)id
        << ", event_type=" << (unsigned int)event_type
        << ", x=" << x
        << ", y=" << y
        << ")";

    emit signalGestureReport(txt);
}

void f6501::OnGestureDxDy(unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type, int period, int dx, int dy, int dist, int angle)
{
    unsigned int ts = int_timestamp();

    // We are not in the gui thread.. send report to gui thread.
    QString txt = stringbuilder()
        << timestamp(ts)
        << " - OnGestureDxDy("
        << " index=" << gi
        << ", id=" << (unsigned int)id
        << ", event_type=" << (unsigned int)event_type
        << ", period=" << period
        << ", dx=" << dx
        << ", dy=" << dy
        << ", dist=" << dist
        << ", angle=" << angle
        << ")";

    emit signalGestureReport(txt);
}

void f6501::OnGestureUnknown(unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type, vector<Byte> v)
{
    unsigned int ts = int_timestamp();

    // We are not in the gui thread.. send report to gui thread.
    stringbuilder txt;

    txt
        << timestamp(ts)
        << " - OnGestureUnknown("
        << " index=" << gi
        << ", id=" << (unsigned int)id
        << ", event_type=" << (unsigned int)event_type
        << ", data=";

    for (auto item : v)
    {
        txt << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)item << " ";
    }

    txt << ")";

    emit signalGestureReport((QString)txt);
}

void f6501::OnGestureFingerStart(unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type, int x, int y, int w, int h)
{
    // We are not in the gui thread.. send report to gui thread.
    QString txt = stringbuilder()
        << "OnGestureFingerStart("
        << " index=" << gi
        << ", id=" << (unsigned int)id
        << ", event_type=" << (unsigned int)event_type
        << ", x=" << x
        << ", y=" << y
        << ", w=" << w
        << ", h=" << h
        << ")";

    emit signalGestureReport(txt);
}

void f6501::OnGestureFingerProgress(unsigned int gi, IFeature6501Gestures::GestureId id, IFeature6501Gestures::EventType event_type, int period, int dX, int dY, int dW, int dH)
{
    // We are not in the gui thread.. send report to gui thread.
    QString txt = stringbuilder()
        << "OnGestureFingerProgress("
        << " index=" << gi
        << ", id=" << (unsigned int)id
        << ", event_type=" << (unsigned int)event_type
        << ", period=" << period
        << ", dX=" << dX
        << ", dY=" << dY
        << ", dW=" << dW
        << ", dH=" << dH
        << ")";

    emit signalGestureReport(txt);
}

string f6501::timestamp(unsigned int int_timestamp)
{
    unsigned int sec = int_timestamp / 1000;
    unsigned int mils = int_timestamp - sec * 1000;

    sec = sec % 60;

    stringbuilder s;
    s << std::setfill('0') << std::setw(2) << sec << "." << std::setfill('0') << std::setw(3) << mils;
    return s;
}

unsigned int f6501::int_timestamp()
{
    using namespace std::chrono;

    auto duration = high_resolution_clock::now().time_since_epoch();
    auto milli = (unsigned int)duration_cast<std::chrono::milliseconds>(duration).count();

    return milli;
}
string f6501::gestureName(IFeature6501Gestures::GestureId gesture)
{
    switch (gesture)
    {
    case IFeature6501Gestures::GestureId::None                         : return "None                         ";
    case IFeature6501Gestures::GestureId::Tap1Finger                   : return "Tap1Finger                   ";
    case IFeature6501Gestures::GestureId::Tap2Finger                   : return "Tap2Finger                   ";
    case IFeature6501Gestures::GestureId::Tap3Finger                   : return "Tap3Finger                   ";
    case IFeature6501Gestures::GestureId::Click1Finger                 : return "Click1Finger                 ";
    case IFeature6501Gestures::GestureId::Click2Finger                 : return "Click2Finger                 ";
    case IFeature6501Gestures::GestureId::Click3Finger                 : return "Click3Finger                 ";
    case IFeature6501Gestures::GestureId::DoubleTap1Finger             : return "DoubleTap1Finger             ";
    case IFeature6501Gestures::GestureId::DoubleTap2Finger             : return "DoubleTap2Finger             ";
    case IFeature6501Gestures::GestureId::DoubleTap3Finger             : return "DoubleTap3Finger             ";
    case IFeature6501Gestures::GestureId::Track1Finger                 : return "Track1Finger                 ";
    case IFeature6501Gestures::GestureId::TrackingAcceleration         : return "TrackingAcceleration         ";
    case IFeature6501Gestures::GestureId::TapDrag1Finger               : return "TapDrag1Finger               ";
    case IFeature6501Gestures::GestureId::TapDrag2Finger               : return "TapDrag2Finger               ";
    case IFeature6501Gestures::GestureId::Drag3Finger                  : return "Drag3Finger                  ";
    case IFeature6501Gestures::GestureId::TapGestures                  : return "TapGestures                  ";
    case IFeature6501Gestures::GestureId::FnClickGestureSuppression    : return "FnClickGestureSuppression    ";
    case IFeature6501Gestures::GestureId::Scroll1Finger                : return "Scroll1Finger                ";
    case IFeature6501Gestures::GestureId::Scroll2Finger                : return "Scroll2Finger                ";
    case IFeature6501Gestures::GestureId::Scroll2FingerHoriz           : return "Scroll2FingerHoriz           ";
    case IFeature6501Gestures::GestureId::Scroll2FingerVert            : return "Scroll2FingerVert            ";
    case IFeature6501Gestures::GestureId::Scroll2FingerStateless       : return "Scroll2FingerStateless       ";
    case IFeature6501Gestures::GestureId::NaturalScrolling             : return "NaturalScrolling             ";
    case IFeature6501Gestures::GestureId::Thumbwheel                   : return "Thumbwheel                   ";
    case IFeature6501Gestures::GestureId::VScrollInertia               : return "VScrollInertia               ";
    case IFeature6501Gestures::GestureId::VScrollBallistics            : return "VScrollBallistics            ";
    case IFeature6501Gestures::GestureId::Swipe2FingerHoriz            : return "Swipe2FingerHoriz            ";
    case IFeature6501Gestures::GestureId::Swipe3FingerHoriz            : return "Swipe3FingerHoriz            ";
    case IFeature6501Gestures::GestureId::Swipe4FingerHoriz            : return "Swipe4FingerHoriz            ";
    case IFeature6501Gestures::GestureId::Swipe3FingerVert             : return "Swipe3FingerVert             ";
    case IFeature6501Gestures::GestureId::Swipe4FingerVert             : return "Swipe4FingerVert             ";
    case IFeature6501Gestures::GestureId::LeftEdgeSwipe1Finger         : return "LeftEdgeSwipe1Finger         ";
    case IFeature6501Gestures::GestureId::RightEdgeSwipe1Finger        : return "RightEdgeSwipe1Finger        ";
    case IFeature6501Gestures::GestureId::BottomEdgeSwipe1Finger       : return "BottomEdgeSwipe1Finger       ";
    case IFeature6501Gestures::GestureId::TopEdgeSwipe1Finger          : return "TopEdgeSwipe1Finger          ";
    case IFeature6501Gestures::GestureId::LeftEdgeSwipe1FingerTrigger  : return "LeftEdgeSwipe1FingerTrigger  ";
    case IFeature6501Gestures::GestureId::RightEdgeSwipe1FingerTrigger : return "RightEdgeSwipe1FingerTrigger ";
    case IFeature6501Gestures::GestureId::BottomEdgeSwipe1FingerTrigger: return "BottomEdgeSwipe1FingerTrigger";
    case IFeature6501Gestures::GestureId::TopEdgeSwipe1FingerTrigger   : return "TopEdgeSwipe1FingerTrigger   ";
    case IFeature6501Gestures::GestureId::LeftEdgeSwipe2Finger         : return "LeftEdgeSwipe2Finger         ";
    case IFeature6501Gestures::GestureId::RightEdgeSwipe2Finger        : return "RightEdgeSwipe2Finger        ";
    case IFeature6501Gestures::GestureId::BottomEdgeSwipe2Finger       : return "BottomEdgeSwipe2Finger       ";
    case IFeature6501Gestures::GestureId::TopEdgeSwipe2Finger          : return "TopEdgeSwipe2Finger          ";
    case IFeature6501Gestures::GestureId::Zoom2Finger                  : return "Zoom2Finger                  ";
    case IFeature6501Gestures::GestureId::Zoom2FingerPinchTrigger      : return "Zoom2FingerPinchTrigger      ";
    case IFeature6501Gestures::GestureId::Zoom2FingerSpreadTrigger     : return "Zoom2FingerSpreadTrigger     ";
    case IFeature6501Gestures::GestureId::Zoom3Finger                  : return "Zoom3Finger                  ";
    case IFeature6501Gestures::GestureId::Zoom2FingerStateless         : return "Zoom2FingerStateless         ";
    case IFeature6501Gestures::GestureId::TwoFingersPresent            : return "TwoFingersPresent            ";
    case IFeature6501Gestures::GestureId::Rotate2Finger                : return "Rotate2Finger                ";
    case IFeature6501Gestures::GestureId::Finger1                      : return "Finger1                      ";
    case IFeature6501Gestures::GestureId::Finger2                      : return "Finger2                      ";
    case IFeature6501Gestures::GestureId::Finger3                      : return "Finger3                      ";
    case IFeature6501Gestures::GestureId::Finger4                      : return "Finger4                      ";
    case IFeature6501Gestures::GestureId::Finger5                      : return "Finger5                      ";
    case IFeature6501Gestures::GestureId::Finger6                      : return "Finger6                      ";
    case IFeature6501Gestures::GestureId::Finger7                      : return "Finger7                      ";
    case IFeature6501Gestures::GestureId::Finger8                      : return "Finger8                      ";
    case IFeature6501Gestures::GestureId::Finger9                      : return "Finger9                      ";
    case IFeature6501Gestures::GestureId::Finger10                     : return "Finger10                     ";
    case IFeature6501Gestures::GestureId::DeviceSpecificRawData        : return "DeviceSpecificRawData        ";
    case IFeature6501Gestures::GestureId::MiddleButton                 : return "MiddleButton                 ";
    case IFeature6501Gestures::GestureId::AppSwitchButton              : return "AppSwitchButton              ";
    case IFeature6501Gestures::GestureId::SmartShiftButton             : return "SmartShiftButton             ";
    default: return stringbuilder() << "Gesture" << (unsigned int)gesture;
    }
}


string f6501::specName(IFeature6501Gestures::SpecId spec)
{
    switch (spec)
    {
    case IFeature6501Gestures::SpecId::DviFieldWidth               : return "DviFieldWidth";
    case IFeature6501Gestures::SpecId::FieldWidths                 : return "FieldWidths";
    case IFeature6501Gestures::SpecId::PeriodUnitInMicroseconds    : return "PeriodUnitInMicroseconds";
    case IFeature6501Gestures::SpecId::Resolution                  : return "Resolution";
    case IFeature6501Gestures::SpecId::Multiplier                  : return "Multiplier";
    case IFeature6501Gestures::SpecId::SensorSizeInPixels          : return "SensorSizeInPixels";
    case IFeature6501Gestures::SpecId::FingerWidthAndHeight        : return "FingerWidthAndHeight";
    case IFeature6501Gestures::SpecId::FingerMajorMinorAngle       : return "FingerMajorMinorAngle";
    case IFeature6501Gestures::SpecId::FingerForcePressure         : return "FingerForcePressure";
    case IFeature6501Gestures::SpecId::Zone                        : return "Zone";
    default: return stringbuilder() << "Spec" << (unsigned int)spec;
    }
}

string f6501::paramName(IFeature6501Gestures::ParamId spec)
{
    switch (spec)
    {
    case IFeature6501Gestures::ParamId::ExtraCapabilites        : return "ExtraCapabilites";
    case IFeature6501Gestures::ParamId::PixelZone               : return "PixelZone";
    case IFeature6501Gestures::ParamId::RatioZone               : return "RatioZone";
    case IFeature6501Gestures::ParamId::ScaleFactor             : return "ScaleFactor";
    default: return stringbuilder() << "Param" << (unsigned int)spec;
    }
}
