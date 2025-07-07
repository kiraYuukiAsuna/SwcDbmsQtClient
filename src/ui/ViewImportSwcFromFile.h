#pragma once

#include <QDialog>
#include <QEvent>
#include <asio.hpp>
#include <filesystem>

#include "src/FileIo/SwcIo.hpp"

struct ExtraSwcImportAttribute {
	std::string m_AnoPath;
	std::string m_ApoPath;
};

QT_BEGIN_NAMESPACE
namespace Ui {
	class ViewImportSwcFromFile;
}
QT_END_NAMESPACE

class MainWindow;

class UpdateImportUiEvent : public QEvent {
public:
	const static Type TYPE = static_cast<Type>(QEvent::User + 1);
	UpdateImportUiEvent(int currentRow, std::string swcFilePath,
						std::string anoFilePath, std::string apoFilePath,
						std::string swcType, int nodeSize, std::string status)
		: QEvent(TYPE),
		  currentRow(currentRow),
		  swcFilePath(swcFilePath),
		  anoFilePath(anoFilePath),
		  apoFilePath(apoFilePath),
		  swcType(swcType),
		  nodeSize(nodeSize),
		  status(status) {}

	int currentRow;
	std::string swcFilePath;
	std::string anoFilePath;
	std::string apoFilePath;
	std::string swcType;
	int nodeSize;
	std::string status;
};

class UpdateImportUiEndEvent : public QEvent {
public:
	const static Type TYPE = static_cast<Type>(QEvent::User + 2);
	UpdateImportUiEndEvent() : QEvent(TYPE) {}
};

class UpdateImportUiErrorEvent : public QEvent {
public:
	const static Type TYPE = static_cast<Type>(QEvent::User + 3);
	UpdateImportUiErrorEvent(std::string errorMessage)
		: QEvent(TYPE), errorMessage(errorMessage) {}

	std::string errorMessage;
};

class ViewImportSwcFromFile : public QDialog {
	Q_OBJECT

public:
	explicit ViewImportSwcFromFile(MainWindow *mainWindow,
								   const std::string &belongToProjectUuid);

	~ViewImportSwcFromFile() override;

	bool event(QEvent *e) override;

private:
	Ui::ViewImportSwcFromFile *ui;
	MainWindow *m_MainWindow;

	std::vector<std::pair<Swc, ExtraSwcImportAttribute>> m_SwcList;
	std::vector<std::pair<ESwc, ExtraSwcImportAttribute>> m_ESwcList;
	bool m_ActionImportComplete = false;

	std::thread m_IoThread;

	void setAllGridColor(int row, const QColor &color);

	std::string convertSwcToUnsorted(const std::filesystem::path &filePath);
};
