#pragma once

#include <QWidget>
#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include "EditorBase.h"


QT_BEGIN_NAMESPACE
namespace Ui { class EditorSwcNode; }
QT_END_NAMESPACE

#include <QAbstractTableModel>

class SwcTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    SwcTableModel(proto::SwcDataV1& swcData, QObject *parent = nullptr)
        : QAbstractTableModel(parent),m_SwcData(swcData)
    {

    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid())
            return 0;

        return m_SwcData.swcdata_size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid())
            return 0;

        // Assuming there are 12 columns according to your code
        return 12;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            QStringList headerLabels;
                headerLabels
                << "n"
                << "type"
                << "x"
                << "y"
                << "z"
                << "radius"
                << "parent"
                << "seg_id"
                << "level"
                << "mode"
                << "timestamp"
                << "feature_value";
            return headerLabels[section];
        }

        return {};
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid())
            return {};

        if (role == Qt::DisplayRole) {
            auto info = m_SwcData.swcdata().Get(index.row());
            const auto& nodeData = info.mutable_swcnodeinternaldata();
            switch (index.column()) {
                case 0: return QString::fromStdString(std::to_string(nodeData->n()));
                case 1: return QString::fromStdString(std::to_string(nodeData->type()));
                case 2: return QString::fromStdString(std::to_string(nodeData->x()));
                case 3: return QString::fromStdString(std::to_string(nodeData->y()));
                case 4: return QString::fromStdString(std::to_string(nodeData->z()));
                case 5: return QString::fromStdString(std::to_string(nodeData->radius()));
                case 6: return QString::fromStdString(std::to_string(nodeData->parent()));
                case 7: return QString::fromStdString(std::to_string(nodeData->seg_id()));
                case 8: return QString::fromStdString(std::to_string(nodeData->level()));
                case 9: return QString::fromStdString(std::to_string(nodeData->mode()));
                case 10: return QString::fromStdString(std::to_string(nodeData->timestamp()));
                case 11: return QString::fromStdString(std::to_string(nodeData->feature_value()));
                default: return {};
            }
        }

        return {};
    }

private:
    proto::SwcDataV1& m_SwcData;
};

class EditorSwcNode : public QWidget, public EditorBase {
Q_OBJECT

public:
    explicit EditorSwcNode(const std::string& swcName, QWidget *parent = nullptr);
    ~EditorSwcNode() override;

    void refreshUserArea();
    void refreshAll();
    void refreshByQueryOption();

    std::string getName() override {
        return m_SwcName;
    }

    MetaInfoType getMetaInfoType() override {
        return MetaInfoType::eSwcData;
    }

    bool save() override{

        return true;
    }

private:
    void loadSwcData(proto::SwcDataV1& swcData);

    Ui::EditorSwcNode *ui;

    std::string m_SwcName;
    proto::SwcDataV1 m_SwcData;
};
