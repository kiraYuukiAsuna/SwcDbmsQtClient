#pragma once

#include <QAbstractTableModel>
#include <QDialog>
#include <google/protobuf/util/time_util.h>
#include "src/framework/service/CachedProtoData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class EditorSwcSnapshot; }
QT_END_NAMESPACE

class SwcSnapshotTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    SwcSnapshotTableModel(std::vector<proto::SwcSnapshotMetaInfoV1>& swcSnapshots, QObject *parent = nullptr)
        : QAbstractTableModel(parent),m_SwcSnapshots(swcSnapshots)
    {

    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid())
            return 0;

        return m_SwcSnapshots.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid())
            return 0;

        return 6;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            QStringList headerLabels;
                headerLabels
                << "CreateTime"
                << "Creator"
                << "SnapshotName"
                << "Id"
                << "DataAccessModelVersion"
                << "Uuid";

            return headerLabels[section];
        }

        return {};
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid())
            return {};

        if (role == Qt::DisplayRole) {
            const auto info = m_SwcSnapshots.at(index.row());
            switch (index.column()) {
                case 0: return QString::fromStdString(google::protobuf::util::TimeUtil::ToString(info.createtime()));
                case 1: return QString::fromStdString(info.creator());
                case 2: return QString::fromStdString(info.swcsnapshotcollectionname());
                case 3: return QString::fromStdString(info.base()._id());
                case 4: return QString::fromStdString(info.base().dataaccessmodelversion());
                case 5: return QString::fromStdString(info.base().uuid());
                default: return {};
            }
        }

        return {};
    }

private:
    std::vector<proto::SwcSnapshotMetaInfoV1>& m_SwcSnapshots;
};

class EditorSwcSnapshot : public QDialog {
Q_OBJECT

public:
    explicit EditorSwcSnapshot(const std::string& swcName, QWidget *parent = nullptr);
    ~EditorSwcSnapshot() override;

private:
    Ui::EditorSwcSnapshot *ui;
    std::string m_SwcName;
    std::vector<proto::SwcSnapshotMetaInfoV1> m_SwcSnapshots;
    void getAllSnapshot();
    void refreshTableView();
};
