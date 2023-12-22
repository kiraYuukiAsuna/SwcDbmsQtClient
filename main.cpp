#include <QApplication>
#include <QPushButton>
#include <iostream>
#include <QFontDatabase>
#include <QProcess>
#include "grpcpp/grpcpp.h"
#include "Service/Service.grpc.pb.h"
#include "src/styles/QtAdvancedStylesheet.h"
#include "src/ui/mainwindow.h"
#include "src/ui/LoginWindow.h"
#include "src/framework/service/RpcCall.h"
#include "src/framework/config/AppConfig.h"
#include "src/framework/defination/ImageDefination.h"

int main(int argc, char *argv[]) {
    setbuf(stdout, nullptr);

    QApplication a(argc, argv);

    const QString fontPath = QString(R"(:/fonts/SourceHanSansCN/SourceHanSansCN-Regular.ttf)");
    const int loadedFontID = QFontDatabase::addApplicationFont(fontPath);
    if (const QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
        !loadedFontFamilies.empty()) {
        const QString& sansCNFamily = loadedFontFamilies.at(0);
        QFont defaultFont = QApplication::font();
        defaultFont.setFamily(sansCNFamily);
        defaultFont.setPixelSize(14);
        QApplication::setFont(defaultFont);
    }

    const QString appDir = QApplication::applicationDirPath();
    acss::QtAdvancedStylesheet styleManager;
    styleManager.setStylesDirPath(R"(:/styles)");
    styleManager.setOutputDirPath(appDir + "/StylesOutput");
    styleManager.setCurrentStyle("qt_material_modified");
    styleManager.setCurrentTheme("light_blue");
    styleManager.updateStylesheet();
    qApp->setStyleSheet(styleManager.styleSheet());
    // setWindowIcon(advancedStyleSheet.styleIcon());
    // qApp->setStyleSheet(advancedStyleSheet.styleSheet());
    // connect(&advancedStyleSheet, SIGNAL(stylesheetChanged()), this,
    //     SLOT(onStyleManagerStylesheetChanged()));

    qApp->setWindowIcon(QIcon(Image::ImageAppIcon));

    AppConfig::getInstance().initialize("AppSecurityConfig.json", "AppConfig.json");
    AppConfig::getInstance().readSecurityConfig();
    AppConfig::getInstance().readConfig();

    auto serverIP = AppConfig::getInstance().getConfig(AppConfig::ConfigItem::eServerIP);
    auto serverPort = AppConfig::getInstance().getConfig(AppConfig::ConfigItem::eServerPort);

    if(serverIP.empty()){
        serverIP = "127.0.0.1";
    }

    if(serverPort.empty()){
        serverPort = "8088";
    }

    auto endPoint = serverIP + ":" + serverPort;

    RpcCall::getInstance().initialize(endPoint);

    if(LoginWindow loginWindow{}; loginWindow.exec() != QDialog::Accepted) {
        return -1;
    }

    MainWindow mainwindow{};
    mainwindow.resize(1920/2, 1080/2);
    mainwindow.show();

    auto exitCode = QApplication::exec();
    if(exitCode == RestartCode) {
        QProcess::startDetached(qApp->applicationFilePath(), QStringList());
    }
}
