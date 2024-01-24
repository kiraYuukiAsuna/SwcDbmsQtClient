#pragma once

#include <QDialog>
#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class ViewCreateSwc; }
QT_END_NAMESPACE

class ViewCreateSwc : public QDialog {
Q_OBJECT

public:
    explicit ViewCreateSwc(QWidget *parent = nullptr);
    ~ViewCreateSwc() override;

private:
    Ui::ViewCreateSwc *ui;
};
