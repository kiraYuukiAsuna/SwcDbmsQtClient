#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class EditorSwcIncrementRecord; }
QT_END_NAMESPACE

class EditorSwcIncrementRecord : public QDialog {
Q_OBJECT

public:
    explicit EditorSwcIncrementRecord(const std::string& swcName, QWidget *parent = nullptr);
    ~EditorSwcIncrementRecord() override;

private:
    Ui::EditorSwcIncrementRecord *ui;
};
