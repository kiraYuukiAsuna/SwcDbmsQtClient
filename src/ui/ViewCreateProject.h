#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class ViewCreateProject; }
QT_END_NAMESPACE

class ViewCreateProject : public QDialog {
Q_OBJECT

public:
    explicit ViewCreateProject(QWidget *parent = nullptr);
    ~ViewCreateProject() override;

private:
    Ui::ViewCreateProject *ui;
};
