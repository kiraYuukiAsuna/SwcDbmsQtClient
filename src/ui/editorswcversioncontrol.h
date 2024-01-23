#pragma once

#include <QDialog>

#include "ViewExportSwcToFile.h"

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

    std::string m_SwcName;

    void getSwcLastSnapshot();
    void getSwcIncrementRecord(proto::SwcSnapshotMetaInfoV1 snapshot);

};

