#pragma once

#include <QDialog>

#include "EditorBase.h"
#include "src/framework/defination/TypeDef.h"
#include "TreeWidget/TreeWidget.h"
#include "Message/Message.pb.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class EditorPermission;
}

QT_END_NAMESPACE

class EditorPermission : public QDialog, public EditorBase {
    Q_OBJECT

public:
    explicit EditorPermission(const std::string&uuid, MetaInfoType type, bool noSaveToCloud, QWidget* parent = nullptr);

    ~EditorPermission() override;

    std::string getName() override {
        return "";
    }

    std::string getUuid() override {
        return "";
    }

    MetaInfoType getMetaInfoType() override {
        return MetaInfoType::ePermissionGroupMetaInfo;
    }

    bool save() override {
        return true;
    }

    void refresh();

    void localRefresh();

    proto::PermissionMetaInfoV1 getPermissionMetaInfo() {
        return m_PermissionMetaInfo;
    }

private:
    Ui::EditorPermission* ui;

    TreeWidget* m_TreeWidget;

    bool m_NoSaveToCloud;

    std::string m_Uuid;
    MetaInfoType m_Type;

    proto::PermissionMetaInfoV1 m_PermissionMetaInfo;
};
