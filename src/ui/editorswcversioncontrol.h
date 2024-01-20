#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class EditorSwcVersionControl; }
QT_END_NAMESPACE

class EditorSwcVersionControl : public QDialog {
Q_OBJECT

public:
    explicit EditorSwcVersionControl(const std::string& swcName, QWidget *parent = nullptr);
    ~EditorSwcVersionControl() override;

private:
    Ui::EditorSwcVersionControl *ui;
};

