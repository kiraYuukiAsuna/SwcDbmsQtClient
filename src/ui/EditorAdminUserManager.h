//
// Created by KiraY on 2023/11/22.
//

#ifndef EDITORADMINUSERMANAGER_H
#define EDITORADMINUSERMANAGER_H

#include <QWidget>

#include "EditorBase.h"


QT_BEGIN_NAMESPACE
namespace Ui { class EditorAdminUserManager; }
QT_END_NAMESPACE

class EditorAdminUserManager : public QWidget, public EditorBase {
Q_OBJECT

public:
    explicit EditorAdminUserManager(QWidget *parent = nullptr);
    ~EditorAdminUserManager() override;

    virtual std::string getName() {
        return "";
    }

    virtual MetaInfoType getMetaInfoType() {
        return MetaInfoType::eUserManagerMetaInfo;
    }
private:
    Ui::EditorAdminUserManager *ui;
};


#endif //EDITORADMINUSERMANAGER_H
