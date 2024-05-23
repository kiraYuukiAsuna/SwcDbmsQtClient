#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class EditorBatchManageSwc; }
QT_END_NAMESPACE

class EditorBatchManageSwc : public QDialog {
Q_OBJECT

public:
    explicit EditorBatchManageSwc(QWidget *parent = nullptr);
    ~EditorBatchManageSwc() override;

private:
    Ui::EditorBatchManageSwc *ui;
};
