#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class EditorAnoAttachment; }
QT_END_NAMESPACE

class EditorAnoAttachment : public QDialog {
Q_OBJECT

public:
    explicit EditorAnoAttachment(const std::string&swcUuid,QWidget *parent = nullptr);
    ~EditorAnoAttachment() override;

private:
    Ui::EditorAnoAttachment *ui;

    std::string m_SwcUuid;
    std::string m_AttachmentUuid;

    bool m_IsAnoAttachmentExist{false};

    void getSwcAnoAttachment();

};

