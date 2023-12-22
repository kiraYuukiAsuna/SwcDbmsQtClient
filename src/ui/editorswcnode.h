#pragma once

#include <QWidget>
#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include "EditorBase.h"


QT_BEGIN_NAMESPACE
namespace Ui { class EditorSwcNode; }
QT_END_NAMESPACE

class EditorSwcNode : public QWidget, public EditorBase {
Q_OBJECT

public:
    explicit EditorSwcNode(const std::string& swcName, QWidget *parent = nullptr);
    ~EditorSwcNode() override;

    void refreshUserArea();
    void refreshTable();
    void refreshAll();
    void refreshByQueryOption();

    virtual std::string getName() {
        return "";
    }

    virtual MetaInfoType getMetaInfoType() {
        return MetaInfoType::eSwcData;
    }

    bool save() override{

        return true;
    }

private:
    void loadSwcData(proto::SwcDataV1& swcData);

    Ui::EditorSwcNode *ui;

    std::string m_SwcName;
    proto::SwcDataV1 m_SwcData;
};
