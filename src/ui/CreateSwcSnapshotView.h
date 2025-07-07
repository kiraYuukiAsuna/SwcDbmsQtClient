#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
	class CreateSwcSnapshotView;
}
QT_END_NAMESPACE

class CreateSwcSnapshotView : public QDialog {
	Q_OBJECT

public:
	explicit CreateSwcSnapshotView(const std::string &swcName,
								   QWidget *parent = nullptr);
	~CreateSwcSnapshotView() override;

private:
	Ui::CreateSwcSnapshotView *ui;
	std::string m_SwcName;
};
