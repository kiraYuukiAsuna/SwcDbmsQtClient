#pragma once

#include <QWidget>
#include <Message/Response.pb.h>

#include "EditorBase.h"


QT_BEGIN_NAMESPACE
namespace Ui { class EditorSwcMetaInfo; }
QT_END_NAMESPACE

class EditorSwcMetaInfo : public QWidget, public EditorBase{
Q_OBJECT

public:
    explicit EditorSwcMetaInfo(proto::GetSwcMetaInfoResponse& response, QWidget *parent = nullptr);
    ~EditorSwcMetaInfo() override;

    std::string getName() override {
        return m_SwcMetaInfo.name();
    }

    MetaInfoType getMetaInfoType() override {
        return MetaInfoType::eSwc;
    }

    bool save() override;

    void refresh(proto::GetSwcMetaInfoResponse& response);

private:
    Ui::EditorSwcMetaInfo *ui;

    proto::SwcMetaInfoV1 m_SwcMetaInfo;
};

