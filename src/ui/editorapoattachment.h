#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class EditorApoAttachment; }
QT_END_NAMESPACE

class EditorApoAttachment : public QDialog {
Q_OBJECT

public:
    explicit EditorApoAttachment(const std::string& swcName,QWidget *parent = nullptr);
    ~EditorApoAttachment() override;

private:
    Ui::EditorApoAttachment *ui;

    std::string m_SwcName;
};
