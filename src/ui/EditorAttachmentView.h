#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class EditorAttachmentView; }
QT_END_NAMESPACE

class EditorAttachmentView : public QDialog {
Q_OBJECT

public:
    explicit EditorAttachmentView(const std::string& swcUuid, const std::string& swcName, QWidget *parent = nullptr);
    ~EditorAttachmentView() override;

private:
    Ui::EditorAttachmentView *ui;

    std::string m_SwcUuid;
    std::string m_SwcName;
};
