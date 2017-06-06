#include "feature.h"
#include "f6010.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"


f6010::f6010(shared_ptr<devio::IFeature6010Gestures> f, Feature* base) :
    QTextEdit(),
    feature6010(f),
    baseFeature(base)
{
    setPlainText(baseFeature->description);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);


    devio::IFeature6010Gestures::Gestures6010 TouchpadFWItemsPresence;
    devio::IFeature6010Gestures::Gestures6010 TouchpadFWItemsDesiredState;
    devio::IFeature6010Gestures::Gestures6010 TouchpadFWItemsState;
    devio::IFeature6010Gestures::Gestures6010 TouchpadFWItemsPermanent;

    bool ok = feature6010->GetTouchpadFWItems(TouchpadFWItemsPresence,TouchpadFWItemsDesiredState,TouchpadFWItemsState,TouchpadFWItemsPermanent);


    append( stringbuilder()
            << " GetTouchpadFWItems("
            << " \nTouchpadFWItemsPresence=" << std::hex<<(short)TouchpadFWItemsPresence.EncodeGesture()
            << " \nTouchpadFWItemsDesiredState=" << std::hex<<(short)TouchpadFWItemsDesiredState.EncodeGesture()
            << " \nTouchpadFWItemsState=" << std::hex<<(short)TouchpadFWItemsState.EncodeGesture()
            << " \nTouchpadFWItemsPermanent=" << std::hex<<(short)TouchpadFWItemsPermanent.EncodeGesture()
            << ") -> " << describe_err(ok)
            );


    append( stringbuilder()
            << " \n\n TouchpadFWItemsPresence => Present/available items in device "
            << " \n Device Capable of Tap (bit0)= "<< TouchpadFWItemsPresence.TapEnable
            << " \n Device Capable of XY ballistics (bit 1)= "<< TouchpadFWItemsPresence.XYBallistics
            << " \n Device Capable of Scrolling ballistics(bit 2)= "<< TouchpadFWItemsPresence.ScrollingBallistics
            << " \n Device Capable of Interia Scrolling(bit 3)= "<< TouchpadFWItemsPresence.InteriaScrolling
            << " \n Device Capable of Edge Scrolling(bit 4)= "<< TouchpadFWItemsPresence.EdgeScrolling
            << " \n Device Capable of Tap Button 15(bit 5)= "<< TouchpadFWItemsPresence.TapButton15
            << " \n Device Capable of Two Finger Right Click(bit 6)= "<< TouchpadFWItemsPresence.TwoFingerRightClick
            << " \n Device Capable of Tap n Hold(bit 7)= "<< TouchpadFWItemsPresence.TapnHold

            << " \n -------------------------- "
            << ") -> "
            );

    append( stringbuilder()
            << " \n\n   => Desired state of items, i.e. what device wants when SW is running (1=enabled by SW, 0=disabled by SW)."
            << " \n Tap (bit0)= "<< TouchpadFWItemsDesiredState.TapEnable
            << " \n XY ballistics (bit 1)= "<< TouchpadFWItemsDesiredState.XYBallistics
            << " \n Scrolling ballistics(bit 2)= "<< TouchpadFWItemsDesiredState.ScrollingBallistics
            << " \n Interia Scrolling(bit 3)= "<< TouchpadFWItemsDesiredState.InteriaScrolling
            << " \n Edge Scrolling(bit 4)= "<< TouchpadFWItemsDesiredState.EdgeScrolling
            << " \n Tap Button 15(bit 5)= "<< TouchpadFWItemsDesiredState.TapButton15
            << " \n Two Finger Right Click(bit 6)= "<< TouchpadFWItemsDesiredState.TwoFingerRightClick
            << " \n Tap n Hold(bit 7)= "<< TouchpadFWItemsDesiredState.TapnHold

            << " \n -------------------------- "
            << ") -> "
            );


    append( stringbuilder()
            << " \n\n   => TouchpadFWItemsState  Currently active items ."
            << " \n Tap (bit0)= "<< TouchpadFWItemsState.TapEnable
            << " \n XY ballistics (bit 1)= "<< TouchpadFWItemsState.XYBallistics
            << " \n Scrolling ballistics(bit 2)= "<< TouchpadFWItemsState.ScrollingBallistics
            << " \n Interia Scrolling(bit 3)= "<< TouchpadFWItemsState.InteriaScrolling
            << " \n Edge Scrolling(bit 4)= "<< TouchpadFWItemsState.EdgeScrolling
            << " \n Tap Button 15(bit 5)= "<< TouchpadFWItemsState.TapButton15
            << " \n Two Finger Right Click(bit 6)= "<< TouchpadFWItemsState.TwoFingerRightClick
            << " \n Tap n Hold(bit 7)= "<< TouchpadFWItemsState.TapnHold

            << " \n -------------------------- "
            << ") -> "
            );


    append( stringbuilder()
            << " \n\n   => TouchpadFWItemsPermanent  Read only. Item is stored permanently (1=permanent/nonvolatile, 0=not permanent/volatile).."
            << " \n Tap (bit0)= "<< TouchpadFWItemsPermanent.TapEnable
            << " \n XY ballistics (bit 1)= "<< TouchpadFWItemsPermanent.XYBallistics
            << " \n Scrolling ballistics(bit 2)= "<< TouchpadFWItemsPermanent.ScrollingBallistics
            << " \n Interia Scrolling(bit 3)= "<< TouchpadFWItemsPermanent.InteriaScrolling
            << " \n Edge Scrolling(bit 4)= "<< TouchpadFWItemsPermanent.EdgeScrolling
            << " \n Tap Button 15(bit 5)= "<< TouchpadFWItemsPermanent.TapButton15
            << " \n Two Finger Right Click(bit 6)= "<< TouchpadFWItemsPermanent.TwoFingerRightClick
            << " \n Tap n Hold(bit 7)= "<< TouchpadFWItemsPermanent.TapnHold

            << " \n -------------------------- "
            << ") -> "
            );


}


