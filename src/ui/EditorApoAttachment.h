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

        return 5; // 减少列数
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            QStringList headerLabels;
            headerLabels
                    << "Error"
                    << "x"
                    << "y"
                    << "z"
                    << "Color"; // 添加颜色列标题
            return headerLabels[section];
        }

        return {};
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid())
            return {};

        auto info = m_SwcAttachmentApoData.at(index.row());
        QColor color(info.colorr(), info.colorg(), info.colorb());

        if (role == Qt::DisplayRole) {
            switch (index.column()) {
                case 0:
                    return QString::fromStdString(info.comment());
                case 1:
                    return QString::fromStdString(std::to_string((int)info.x()));
                case 2:
                    return QString::fromStdString(std::to_string((int)info.y()));
                case 3:
                    return QString::fromStdString(std::to_string((int)info.z()));
                case 4:
                    return ""; // 显示颜色的名称
                default:
                    return {};
            }
        } else if (role == Qt::BackgroundRole && index.column() == 4) {
            return color; // 设置背景颜色
        }

        return {};
    }

private:
    std::vector<proto::SwcAttachmentApoV1> &m_SwcAttachmentApoData;
};
class EditorApoAttachment : public QDialog {
Q_OBJECT

public:
    explicit EditorApoAttachment(const std::string& swcUuid, QWidget *parent = nullptr);

    ~EditorApoAttachment() override;

private:
    Ui::EditorApoAttachment *ui;

    std::string m_SwcUuid;
    std::string m_AttachmentUuid;

    bool m_IsApoAttachmentExist{false};

    std::vector<proto::SwcAttachmentApoV1> m_SwcAttachmentApoData;

    void getSwcApoAttachment();

    void loadSwcAttachmentApoData();
};
