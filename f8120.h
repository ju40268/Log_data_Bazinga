#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include "feature.h"
#include "devio.h"
#include <vector>

using devio::IFeature;
using devio::IFeature8120GamingAttachments;
using devio::Byte;
using devio::Subscriber;
using std::vector;


class f8120 : public QTextEdit, public Subscriber < devio::IFeature8120GamingAttachments::Report >
{
    Q_OBJECT

public:
    explicit f8120(shared_ptr<devio::IFeature8120GamingAttachments> f, Feature *base);

    virtual void onAttachmentsChanged(const std::vector<IFeature8120GamingAttachments::AttachmentId>& attachments);

private:
    void createUI(void);
    void notifyText(const QString &text, bool insertCRLF = false);
    std::string attachmentName(IFeature8120GamingAttachments::AttachmentId);

    shared_ptr<devio::IFeature8120GamingAttachments> feature8120;
    Feature *baseFeature;
};
