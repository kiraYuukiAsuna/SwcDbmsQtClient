#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class EditorPermission; }
QT_END_NAMESPACE

class EditorPermission : public QDialog {
Q_OBJECT

public:
    explicit EditorPermission(QWidget *parent = nullptr);
    ~EditorPermission() override;

private:
    Ui::EditorPermission *ui;
};

