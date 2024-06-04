#pragma once

#include <QAbstractTableModel>
#include <QDialog>
#include <google/protobuf/util/time_util.h>

#include "ViewExportSwcToFile.h"

QT_BEGIN_NAMESPACE
namespace Ui { class EditorSwcIncrementRecord; }
QT_END_NAMESPACE

class SwcIncrementRecordTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    SwcIncrementRecordTableModel(std::vector<proto::SwcIncrementOperationMetaInfoV1>& swcIncrements, QObject *parent = nullptr)
        : QAbstractTableModel(parent),m_SwcIncrements(swcIncrements)
    {

    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid())
            return 0;

        return m_SwcIncrements.size();
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
                << "BasedSnapshot"
                << "IncrementOperationRecordName"
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
            const auto info = m_SwcIncrements.at(index.row());
            switch (index.column()) {
                case 0: return QString::fromStdString(google::protobuf::util::TimeUtil::ToString(info.createtime()));
                case 1: return QString::fromStdString(info.startsnapshot());
                case 2: return QString::fromStdString(info.incrementoperationcollectionname());
                case 3: return QString::fromStdString(info.base()._id());
                case 4: return QString::fromStdString(info.base().dataaccessmodelversion());
                case 5: return QString::fromStdString(info.base().uuid());
                default: return {};
            }
        }

        return {};
    }

private:
    std::vector<proto::SwcIncrementOperationMetaInfoV1>& m_SwcIncrements;
};

class EditorSwcIncrementRecord : public QDialog {
Q_OBJECT

public:
    explicit EditorSwcIncrementRecord(const std::string& swcUuid, QWidget *parent = nullptr);
    ~EditorSwcIncrementRecord() override;

private:
    Ui::EditorSwcIncrementRecord *ui;

    std::string m_SwcUuid;
    std::vector<proto::SwcIncrementOperationMetaInfoV1> m_SwcIncrements;
    void getAllSwcIncrementRecord();
    void refreshTableView();
};
