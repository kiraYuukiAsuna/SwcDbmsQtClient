#pragma once

#include <QDialog>

#include "ViewExportSwcToFile.h"
#include <QtNodes/GraphicsView>
#include <QtNodes/DataFlowGraphicsScene>

QT_BEGIN_NAMESPACE
namespace Ui { class EditorSwcVersionControl; }
QT_END_NAMESPACE

class EditorSwcVersionControl : public QDialog {
Q_OBJECT

public:
    explicit EditorSwcVersionControl(const std::string &swcName, QWidget *parent = nullptr);

    ~EditorSwcVersionControl() override;

private:
    Ui::EditorSwcVersionControl *ui;

    std::string m_SwcName;

    bool m_HasExportedStatus{false};
    int64_t m_EndTime;
    std::string m_SnapshotName;
    std::string m_IncremrntOpName;

    std::vector<proto::SwcSnapshotMetaInfoV1> m_SwcSnapshots;
    std::vector<proto::SwcIncrementOperationMetaInfoV1> m_SwcIncrements;

    void getSwcLastSnapshot();

    void getSwcIncrementRecord(proto::SwcSnapshotMetaInfoV1 snapshot, int64_t endTime);

    void promoteOperation(std::vector<proto::SwcNodeDataV1> &nodeData, const proto::SwcIncrementOperationV1 &op,
                          int64_t endTime);

    void getAllSnapshot();

    void getAllSwcIncrementRecord();

    QtNodes::DataFlowGraphModel m_DataFlowGraphModel;
    QtNodes::GraphicsView m_GraphicsView;
    QtNodes::DataFlowGraphicsScene *m_DataFlowGraphicsScene;
};

