#pragma once

#include <QAbstractTableModel>
#include <QDialog>

#include "ViewSwcNodeData.h"

class SwcAttachmentTableModel : public QAbstractTableModel {
	Q_OBJECT

public:
	SwcAttachmentTableModel(std::vector<proto::SwcNodeDataV1> &swcData,
							QObject *parent = nullptr)
		: QAbstractTableModel(parent), m_SwcData(swcData) {}

	int rowCount(const QModelIndex &parent = QModelIndex()) const override {
		if (parent.isValid()) return 0;

		return m_SwcData.size();
	}

	int columnCount(const QModelIndex &parent = QModelIndex()) const override {
		if (parent.isValid()) return 0;

		return 12;
	}

	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const override {
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
			QStringList headerLabels;
			headerLabels << "n"
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

	QVariant data(const QModelIndex &index,
				  int role = Qt::DisplayRole) const override {
		if (!index.isValid()) return {};

		if (role == Qt::DisplayRole) {
			auto info = m_SwcData.at(index.row());
			const auto &nodeData = info.mutable_swcnodeinternaldata();
			switch (index.column()) {
				case 0:
					return QString::fromStdString(
						std::to_string(nodeData->n()));
				case 1:
					return QString::fromStdString(
						std::to_string(nodeData->type()));
				case 2:
					return QString::fromStdString(
						std::to_string(nodeData->x()));
				case 3:
					return QString::fromStdString(
						std::to_string(nodeData->y()));
				case 4:
					return QString::fromStdString(
						std::to_string(nodeData->z()));
				case 5:
					return QString::fromStdString(
						std::to_string(nodeData->radius()));
				case 6:
					return QString::fromStdString(
						std::to_string(nodeData->parent()));
				case 7:
					return QString::fromStdString(
						std::to_string(nodeData->seg_id()));
				case 8:
					return QString::fromStdString(
						std::to_string(nodeData->level()));
				case 9:
					return QString::fromStdString(
						std::to_string(nodeData->mode()));
				case 10:
					return QString::fromStdString(
						std::to_string(nodeData->timestamp()));
				case 11:
					return QString::fromStdString(
						std::to_string(nodeData->feature_value()));
				default:
					return {};
			}
		}

		return {};
	}

private:
	std::vector<proto::SwcNodeDataV1> &m_SwcData;
};

QT_BEGIN_NAMESPACE
namespace Ui {
	class EditorSwcAttachment;
}
QT_END_NAMESPACE

class EditorSwcAttachment : public QDialog {
	Q_OBJECT

public:
	explicit EditorSwcAttachment(const std::string &swcUuid,
								 QWidget *parent = nullptr);
	~EditorSwcAttachment() override;

private:
	Ui::EditorSwcAttachment *ui;

	std::string m_SwcUuid;
	std::string m_SwcAttachmentUuid;

	bool m_IsSwcAttachmentExist{false};

	std::vector<proto::SwcNodeDataV1> m_SwcAttachmentSwcData;

	void getSwcAttachmentSwc();

	void loadSwcAttachmentSwcData();
};
