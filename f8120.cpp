#include "feature.h"
#include "f8120.h"
#include "xlog/xlog.h"
#include "stringbuilder.h"

using devio::Delivery;

f8120::f8120(shared_ptr<devio::IFeature8120GamingAttachments> f, Feature* base) :
    QTextEdit(),
    feature8120(f),
    baseFeature(base)
{
    // Test...
    std::vector<IFeature8120GamingAttachments::AttachmentId> supported = f->supportedAttachments();
    std::vector<IFeature8120GamingAttachments::AttachmentId> attached = f->currentAttachments();

    // Subscribe to all
    bool b = f->subscribe(attached);

    setPlainText(baseFeature->description);
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);

    notifyText(stringbuilder() << "Supported Attachments");
    for(auto id : supported)
    {
        notifyText(stringbuilder() << "    " << attachmentName(id));
    }

    notifyText(stringbuilder() << "Current Attachments");
    for (auto id : attached)
    {
        notifyText(stringbuilder() << "    " << attachmentName(id));
    }

    subscribe(f, Delivery::Immediate);
}


//***************************** ********************************************
//
// f8120::onAttachmentsChanged
//
//*************************************************************************

void f8120::onAttachmentsChanged(const std::vector<IFeature8120GamingAttachments::AttachmentId>& attachments)
{
    notifyText(stringbuilder() << "Attachments Changed");
    for (auto id : attachments)
    {
        notifyText(stringbuilder() << "    " << attachmentName(id));
    }
}


//*************************************************************************
//
// f8120::attachmentName
//
//*************************************************************************

std::string f8120::attachmentName(IFeature8120GamingAttachments::AttachmentId id)
{
    switch (id)
    {
        case IFeature8120GamingAttachments::DaylightStarlightPedals:
            return "DaylightStarlightPedals";
        case IFeature8120GamingAttachments::ExternalPower:
            return "ExternalPower";
        case IFeature8120GamingAttachments::DaylightStarlightShifter:
            return "DaylightStarlightShifter";
        case IFeature8120GamingAttachments::SnowballShifter:
            return "SnowballShifter";
        default:
            return "Unknown";
    }
}

//*************************************************************************
//
// f8120::notifyText
//
//*************************************************************************

void f8120::notifyText(const QString &text, bool insertCRLF)
{
    // Jump to the end (to restore the cursor)
    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    setTextCursor(tc);

    insertPlainText(text + QString("\n"));
    if (insertCRLF)
    {
        append("\n");
    }

    // Jump to the end (to scroll to the end)
    tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    setTextCursor(tc);
}
