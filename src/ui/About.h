#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE

namespace Ui {
    class About;
}

QT_END_NAMESPACE

class About : public QDialog {
    Q_OBJECT

public:
    explicit About(QWidget* parent = nullptr);

    ~About() override;

private:
    Ui::About* ui;
};
