#pragma once

#include <qprogressbar.h>

#include <QDialog>
#include <QLabel>

class ProgressBar : public QDialog {
	Q_OBJECT

public:
	ProgressBar(QWidget* parent = nullptr);
	~ProgressBar();

	void setText(const std::string& text);
	void setValue(int value);

	void finish();

private:
	QProgressBar* m_ProgressBar;
	QLabel* m_TitleLabel;
	QLabel* m_ProgressLabel;
};
