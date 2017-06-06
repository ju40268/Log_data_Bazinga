#include "feature.h"
#include "f6500.h"
#include <sstream>
#include "stringbuilder.h"

using std::string;
using devio::Delivery;

f6500::f6500(shared_ptr<devio::IFeature6500Gestures> f, Feature* base) :
    QTextEdit(),
    feature6500(f),
    baseFeature(base)
{
    setPlainText(baseFeature->description);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);

    int count=0;
    bool result = feature6500->GetGestureCount( &count);

    append(stringbuilder()
            << " GetGestureCount("
            << " count=" << count
            << ") -> " << result
           );

    for( int i =0; i < count; i++ )
    {
        int gesture_id =0;
        int task_id =0;
        int flags =0;
        result = feature6500->GetGestureIDInfo(i,&gesture_id,&task_id,&flags);

        append(stringbuilder()
               << " GetGestureIDInfo("
               << " gesture_id=0x" << std::hex << gesture_id
               << " task_id=0x" << task_id
               << " flags=0x" << flags
               << ") -> " << result
               );

// CG: We need to figure out what the tool is supposed to do when the 0x6500 feature gets selected.
//
//     For now, we are just listening to 0x6500 scroll notifications, and we are not changing the
//     reporting type of any gestures (no reporting, HID, HID++ or both).
//
//     Scroll notifications will only be reported if the scroll gesture has been enabled for HID++
//     reporting (which Akebono does).
//
//     We could enable gestures only when the window is focused for example, and restore the gestures
//     when focus goes to another window, to prevent conflicts with Akebono.

//        // we don't want to enable HID++ reporting only for Track1Finger gesture
//        if(gesture_id != 0x7701)
//        {
//            int enable = 2;
//            result = feature6500->Enable((short)gesture_id,enable);
//
//            append(stringbuilder()
//                   << " Enable("
//                   << " gesture_id=0x" << std::hex << gesture_id
//                   << " enable=" << std::dec << enable
//                   << ") -> " << result
//                   );
//        }
    }

    // need to register this to use this type in a signal
    qRegisterMetaType<GestureId>("GestureId");
    qRegisterMetaType<EventType>("EventType");

    bool connected = connect(this, SIGNAL(signalGestureScrollEvent(GestureId, EventType, int, int, int)),
                             this, SLOT(slotGestureScrollEvent(GestureId, EventType, int, int, int)),
                             Qt::QueuedConnection);
    if (!connected)
    {
        append("Error: Signal not connected!");
    }

    bool bresult = subscribe(feature6500,Delivery::Immediate);
    append(stringbuilder() 
        << "Subscribing to 6500 reports -> " 
        << describe_err(bresult)
        );
}

f6500::~f6500()
{
// Commented this out for now. See comment above in constructor.
//
//    int count=0;
//    bool result = feature6500->GetGestureCount( &count);
//
//    for( int i =0; i < count; i++ )
//    {
//        int gesture_id =0;
//        int task_id =0;
//        int flags =0;
//        result = feature6500->GetGestureIDInfo(i,&gesture_id,&task_id,&flags);
//
//        if(gesture_id != 0x7701)
//        {
//            // 1 => report as HID (or native function)
//            int enable = 1;
//            feature6500->Enable((short)gesture_id,enable);
//        }
//    }
}

void f6500::slotGestureScrollEvent(GestureId gesture_id, EventType event_type, int dx, int dy, int period )
{
    append(stringbuilder()
           << " OnGestureScrollEvent("
           << " gesture_id=0x" << std::hex << int(gesture_id)
           << ", event_type=" << std::dec << int(event_type)
           << ", dx=" << std::dec << dx
           << ", dy=" << dy
           << ", period=" << period
           );
}

void f6500::OnGestureScrollEvent(GestureId gesture_id, EventType event_type, int dx, int dy, int period )
{
    emit signalGestureScrollEvent(gesture_id, event_type, dx, dy, period );
}
