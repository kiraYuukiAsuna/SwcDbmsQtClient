//
// Created by KiraY on 2024/1/23.
//

#ifndef VIEWINCREMENTRECORD_H
#define VIEWINCREMENTRECORD_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
	class ViewIncrementRecord;
}
QT_END_NAMESPACE

class ViewIncrementRecord : public QDialog {
	Q_OBJECT

public:
	explicit ViewIncrementRecord(QWidget *parent = nullptr);
	~ViewIncrementRecord() override;

private:
	Ui::ViewIncrementRecord *ui;
};

#endif	// VIEWINCREMENTRECORD_H
