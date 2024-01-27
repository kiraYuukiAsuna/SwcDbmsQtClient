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

    bool m_HasExportedStatus{false};
    int64_t m_EndTime;
    std::string m_SnapshotName;
    std::string m_IncremrntOpName;

    void getSwcLastSnapshot();
    void getSwcIncrementRecord(proto::SwcSnapshotMetaInfoV1 snapshot, int64_t endTime);

    void promoteOperation(std::vector<proto::SwcNodeDataV1>& nodeData, const proto::SwcIncrementOperationV1& op,int64_t endTime);

};

