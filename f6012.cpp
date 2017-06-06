#include "feature.h"
#include "f6012.h"
#include <sstream>
#include <iomanip>
#include "stringbuilder.h"


f6012::f6012(shared_ptr<devio::IFeature6012Gestures> f, Feature* base) :
    QTextEdit(),
    feature6012(f),
    baseFeature(base)
{
    setPlainText(baseFeature->description);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);


    devio::IFeature6012Gestures::Gestures6012 TouchpadWin8FWItemsPresence;
    devio::IFeature6012Gestures::Gestures6012 TouchpadWin8FWItemsDesiredState;
    devio::IFeature6012Gestures::Gestures6012 TouchpadWin8FWItemsState;
    devio::IFeature6012Gestures::Gestures6012 TouchpadWin8FWItemsPermanent;

    bool ok = feature6012->GetTouchpadWin8FWItems(TouchpadWin8FWItemsPresence,TouchpadWin8FWItemsDesiredState,TouchpadWin8FWItemsState,TouchpadWin8FWItemsPermanent);


    append( stringbuilder()
            << " GetTouchpadWin8FWItems("
            << " \nTouchpadWin8FWItemsPresence=" << std::hex<<(short)TouchpadWin8FWItemsPresence.EncodeGesture()
            << " \nTouchpadWin8FWItemsDesiredState=" << std::hex<<(short)TouchpadWin8FWItemsDesiredState.EncodeGesture()
            << " \nTouchpadWin8FWItemsState=" << std::hex<<(short)TouchpadWin8FWItemsState.EncodeGesture()
            << " \nTouchpadWin8FWItemsPermanent=" << std::hex<<(short)TouchpadWin8FWItemsPermanent.EncodeGesture()
            << ") -> " << describe_err(ok)
            );


    append( stringbuilder()
            << " \n\n TouchpadWin8FWItemsPresence => Present/available items in device "
            << " \n Device Capable of Left edge swipe gesture (bit0)= "<< TouchpadWin8FWItemsPresence.LeftEdgeSwipe
            << " \n Device Capable of Right edge swipe gesture.(bit 1)= "<< TouchpadWin8FWItemsPresence.RightEdgeSwipe
            << " \n Device Capable of Top edge swipe gesture(bit 2)= "<< TouchpadWin8FWItemsPresence.TopEdgeSwipe
            << " \n Device Capable of Two Finger Zoom (bit 3)= "<< TouchpadWin8FWItemsPresence.TwoFingerZoom
            << " \n Reserved (bit 4)= "
            << " \n Reserved  (bit 5)= "
            << " \n Reserved (bit 6)= "
            << " \n Reserved (bit 7)= "

            << " \n -------------------------- "
            << ") -> "
            );

    append( stringbuilder()
            << " \n\n   => Desired state of items, i.e. what device wants when SW is running (1=enabled by SW, 0=disabled by SW)."
            << " \n Left edge swipe gesture (bit0)= "<< TouchpadWin8FWItemsDesiredState.LeftEdgeSwipe
            << " \n Right edge swipe gesture(bit 1)= "<< TouchpadWin8FWItemsDesiredState.RightEdgeSwipe
            << " \n Top edge swipe gesture(bit 2)= "<< TouchpadWin8FWItemsDesiredState.TopEdgeSwipe
            << " \n Two Finger Zoom (bit 3)= "<< TouchpadWin8FWItemsDesiredState.TwoFingerZoom
            << " \n Reserved (bit 4)= "
            << " \n Reserved (bit 5)= "
            << " \n Reserved (bit 6)= "
            << " \n Reserved (bit 7)= "

            << " \n -------------------------- "
            << ") -> "
            );


    append( stringbuilder()
            << " \n\n   => TouchpadWin8FWItemsState  Currently active items ."
            << " \n Left edge swipe gesture (bit0)= "<< TouchpadWin8FWItemsDesiredState.LeftEdgeSwipe
            << " \n Right edge swipe gesture(bit 1)= "<< TouchpadWin8FWItemsDesiredState.RightEdgeSwipe
            << " \n Top edge swipe gesture(bit 2)= "<< TouchpadWin8FWItemsDesiredState.TopEdgeSwipe
            << " \n Two Finger Zoom (bit 3)= "<< TouchpadWin8FWItemsDesiredState.TwoFingerZoom
            << " \n Reserved (bit 4)= "
            << " \n Reserved (bit 5)= "
            << " \n Reserved (bit 6)= "
            << " \n Reserved (bit 7)= "

            << " \n -------------------------- "
            << ") -> "
            );

    append( stringbuilder()
            << " \n\n   => TouchpadFWItemsPermanent  Read only. Item is stored permanently (1=permanent/nonvolatile, 0=not permanent/volatile).."
            << " \n Left edge swipe gesture (bit0)= "<< TouchpadWin8FWItemsPermanent.LeftEdgeSwipe
            << " \n Right edge swipe gesture(bit 1)= "<< TouchpadWin8FWItemsPermanent.RightEdgeSwipe
            << " \n Top edge swipe gesture(bit 2)= "<< TouchpadWin8FWItemsPermanent.TopEdgeSwipe
            << " \n Two Finger Zoom (bit 3)= "<< TouchpadWin8FWItemsPermanent.TwoFingerZoom
            << " \n Reserved (bit 4)= "
            << " \n Reserved (bit 5)= "
            << " \n Reserved (bit 6)= "
            << " \n Reserved (bit 7)= "

            << " \n -------------------------- "
            << ") -> "
            );


}


