#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTimer>

#include "LeftClientView.h"
#include "RightClientView.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow() override;

	LeftClientView& getLeftClientView();
	RightClientView& getRightClientView();

private:
	QTimer* m_HeartBeatTimer;
	QTimer* m_OnlineStatusTimer;

	QSplitter* m_Splitter;
	LeftClientView* m_LeftClientView;
	RightClientView* m_RightClientView;
};
