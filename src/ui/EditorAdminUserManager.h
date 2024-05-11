#pragma once

#include <QWidget>

#include "EditorBase.h"


QT_BEGIN_NAMESPACE

namespace Ui {
    class EditorAdminUserManager;
}

QT_END_NAMESPACE

class EditorAdminUserManager : public QWidget, public EditorBase {
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

private:
    Ui::EditorAdminUserManager* ui;
};
