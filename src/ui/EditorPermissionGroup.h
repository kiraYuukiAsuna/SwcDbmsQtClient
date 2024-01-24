//
// Created by KiraY on 2023/11/23.
//

#ifndef EDITORPERMISSIONGROUP_H
#define EDITORPERMISSIONGROUP_H

#include <QWidget>

#include "EditorBase.h"


QT_BEGIN_NAMESPACE
namespace Ui { class EditorPermissionGroup; }
QT_END_NAMESPACE

class EditorPermissionGroup : public QWidget, public EditorBase{
Q_OBJECT

public:
    explicit EditorPermissionGroup(QWidget *parent = nullptr);
    ~EditorPermissionGroup() override;

    virtual std::string getName() {
        return "";
    }

    virtual MetaInfoType getMetaInfoType() {
        return MetaInfoType::ePermissionGroupMetaInfo;
    }
private:
    Ui::EditorPermissionGroup *ui;
};


#endif //EDITORPERMISSIONGROUP_H
