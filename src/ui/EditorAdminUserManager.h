#pragma once

#include <QDialog>
#include "EditorBase.h"
#include "TreeWidget/TreeWidget.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class EditorAdminUserManager;
}

QT_END_NAMESPACE

class EditorAdminUserManager : public QDialog, public EditorBase {
    Q_OBJECT

public:
    explicit EditorAdminUserManager(QWidget* parent = nullptr);

    ~EditorAdminUserManager() override;

    std::string getName() override {
        return "";
    }

    MetaInfoType getMetaInfoType() override {
        return MetaInfoType::eUserManagerMetaInfo;
    }

    bool save() override {
        return true;
    }

    void refresh();

private:
    Ui::EditorAdminUserManager* ui;

    TreeWidget* m_TreeWidget;
};
