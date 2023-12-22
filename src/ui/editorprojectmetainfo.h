#pragma once

#include <QWidget>
#include <Message/Request.pb.h>
#include <Message/Response.pb.h>

#include "EditorBase.h"
#include "src/framework/service/CachedProtoData.h"


QT_BEGIN_NAMESPACE
namespace Ui { class EditorProjectMetaInfo; }
QT_END_NAMESPACE

class EditorProjectMetaInfo : public QWidget, public EditorBase{
Q_OBJECT

public:
    explicit EditorProjectMetaInfo(proto::GetProjectResponse& response, QWidget *parent = nullptr);
    ~EditorProjectMetaInfo() override;

    std::string getName() override {
        return m_ProjectMetaInfo.name();
    }

    MetaInfoType getMetaInfoType() override {
        return MetaInfoType::eProject;
    }

    bool save() override;

    void refresh(proto::GetProjectResponse& response);

private:
    Ui::EditorProjectMetaInfo *ui;

    proto::ProjectMetaInfoV1 m_ProjectMetaInfo;
};

