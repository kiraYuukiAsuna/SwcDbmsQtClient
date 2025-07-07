#pragma once

#include <QDialog>
#include <thread>

#include "Message/Message.pb.h"

struct ExportSwcData {
	proto::SwcMetaInfoV1 swcMetaInfo;
	proto::SwcDataV1 swcData;
	bool isSnapshot{false};
	std::string swcSnapshotCollectionName;
};

QT_BEGIN_NAMESPACE
namespace Ui {
	class ViewExportSwcToFile;
}
QT_END_NAMESPACE

class ViewExportSwcToFile : public QDialog {
	Q_OBJECT

public:
	explicit ViewExportSwcToFile(std::vector<ExportSwcData> &exportSwcData,
								 bool getDataFromServer,
								 QWidget *parent = nullptr);
	~ViewExportSwcToFile() override;

private:
	Ui::ViewExportSwcToFile *ui;
	bool m_GetDataFromServer = false;
	std::vector<ExportSwcData> m_ExportSwcData;

	std::string m_SavePath;
	bool m_ActionExportComplete = false;

	std::thread m_IoThread;

	void setAllGridColor(int row, QColor color);
};
