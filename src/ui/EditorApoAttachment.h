#pragma once

#include <QAbstractTableModel>
#include <QDialog>

#include "ViewExportSwcToFile.h"


QT_BEGIN_NAMESPACE

namespace Ui {
    class EditorApoAttachment;
}

QT_END_NAMESPACE

class SwcAttachmentApoTableModel : public QAbstractTableModel {
Q_OBJECT

public:
    SwcAttachmentApoTableModel(std::vector<proto::SwcAttachmentApoV1> &swcData, QObject *parent = nullptr)
            : QAbstractTableModel(parent), m_SwcAttachmentApoData(swcData) {
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid())
            return 0;

        return m_SwcAttachmentApoData.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid())
            return 0;

        return 15;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            QStringList headerLabels;
            headerLabels
                    << "n"
                    << "orderinfo"
                    << "name"
                    << "comment"
                    << "z"
                    << "x"
                    << "y"
                    << "pixmax"
                    << "intensity"
                    << "sdev"
                    << "volsize"
                    << "mass"
                    << "color_r"
                    << "color_g"
                    << "color_b";
            return headerLabels[section];
        }

        return {};
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid())
            return {};

        if (role == Qt::DisplayRole) {
            auto info = m_SwcAttachmentApoData.at(index.row());
            switch (index.column()) {
                case 0:
                    return QString::fromStdString(std::to_string(info.n()));
                case 1:
                    return QString::fromStdString(info.orderinfo());
                case 2:
                    return QString::fromStdString(info.name());
                case 3:
                    return QString::fromStdString(info.comment());
                case 4:
                    return QString::fromStdString(std::to_string(info.z()));
                case 5:
                    return QString::fromStdString(std::to_string(info.x()));
                case 6:
                    return QString::fromStdString(std::to_string(info.y()));
                case 7:
                    return QString::fromStdString(std::to_string(info.pixmax()));
                case 8:
                    return QString::fromStdString(std::to_string(info.intensity()));
                case 9:
                    return QString::fromStdString(std::to_string(info.sdev()));
                case 10:
                    return QString::fromStdString(std::to_string(info.volsize()));
                case 11:
                    return QString::fromStdString(std::to_string(info.mass()));
                case 12:
                    return QString::fromStdString(std::to_string(info.colorr()));
                case 13:
                    return QString::fromStdString(std::to_string(info.colorg()));
                case 14:
                    return QString::fromStdString(std::to_string(info.colorb()));
                default:
                    return {};
            }
        }

        return {};
    }

private:
    std::vector<proto::SwcAttachmentApoV1> &m_SwcAttachmentApoData;
};

class EditorApoAttachment : public QDialog {
Q_OBJECT

public:
    explicit EditorApoAttachment(const std::string &swcName, QWidget *parent = nullptr);

    ~EditorApoAttachment() override;

private:
    Ui::EditorApoAttachment *ui;

    std::string m_SwcName;
    std::string m_AttachmentUuid;

    bool m_IsApoAttachmentExist{false};

    std::vector<proto::SwcAttachmentApoV1> m_SwcAttachmentApoData;

    void getSwcApoAttachment();

    void loadSwcAttachmentApoData();
};
