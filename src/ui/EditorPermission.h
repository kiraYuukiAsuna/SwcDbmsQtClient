#pragma once

#include <QDialog>

#include "EditorBase.h"
#include "src/framework/defination/TypeDef.h"
#include "TreeWidget/TreeWidget.h"
#include "Message/Message.pb.h"

QT_BEGIN_NAMESPACE
namespace Ui { class EditorPermission; }
QT_END_NAMESPACE

class EditorPermission : public QDialog, public EditorBase{
Q_OBJECT

public:
    explicit EditorPermission(const std::string& name, MetaInfoType type, QWidget *parent = nullptr);
    ~EditorPermission() override;

    std::string getName() override {
        return "";
    }

    MetaInfoType getMetaInfoType() override {
        return MetaInfoType::ePermissionGroupMetaInfo;
    }

    bool save() override {
        return true;
    }

    void refresh();

private:
    Ui::EditorPermission *ui;

    TreeWidget* m_TreeWidget;

    std::string m_Name;
    MetaInfoType m_Type;

    proto::PermissionMetaInfoV1 m_PermissionMetaInfo;
};

