#pragma once

#include <QDialog>
#include "src/FileIo/SwcIo.hpp"


struct ExtraSwcImportAttribute {
    std::string m_AnoPath;
    std::string m_ApoPath;
};

QT_BEGIN_NAMESPACE
namespace Ui { class ViewImportSwcFromFile; }
QT_END_NAMESPACE

class MainWindow;

class ViewImportSwcFromFile : public QDialog {
Q_OBJECT

public:
    explicit ViewImportSwcFromFile(MainWindow *mainWindow);

    ~ViewImportSwcFromFile() override;

private:
    Ui::ViewImportSwcFromFile *ui;
    MainWindow *m_MainWindow;

    std::vector<std::pair<Swc, ExtraSwcImportAttribute>> m_SwcList;
    std::vector<std::pair<ESwc, ExtraSwcImportAttribute>> m_ESwcList;
    bool m_ActionImportComplete = false;

    void setAllGridColor(int row, const QColor &color);

};
