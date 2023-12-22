#pragma once

#include <QDialog>
#include "src/swcio/SwcIO.h"


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
    MainWindow* m_MainWindow;

    std::vector<Swc> m_SwcList;
    std::vector<ESwc> m_ESwcList;
    bool m_ActionImportComplete= false;

};
