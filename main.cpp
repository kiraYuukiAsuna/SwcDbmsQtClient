#include <QApplication>
#include <QFontDatabase>
#include <QProcess>
#include "src/styles/QtAdvancedStylesheet.h"
#include "src/ui/MainWindow.h"
#include "src/ui/LoginWindow.h"
#include "src/framework/service/RpcCall.h"
#include "src/framework/config/AppConfig.h"
#include "src/framework/core/log/Log.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"
#include "src/framework/util/util.hpp"

int main(int argc, char* argv[]) {
    Seele::Log::Init("logs", "Core.log", "App.log", "EditorConsole.log", true, false);

    auto timestamp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()).
            time_since_epoch();
    SeeleInfoTag("Main", "App Start At: {}", timestampToString(timestamp.count()));

    QApplication app(argc, argv);

    const QString fontPath = QString(R"(:/fonts/SourceHanSansCN/SourceHanSansCN-Regular.ttf)");
    const int loadedFontID = QFontDatabase::addApplicationFont(fontPath);
    if (const QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
        !loadedFontFamilies.empty()) {
        const QString&sansCNFamily = loadedFontFamilies.at(0);
        QFont defaultFont = QApplication::font();
        defaultFont.setFamily(sansCNFamily);
        defaultFont.setPixelSize(20);
        defaultFont.setBold(true);
        QApplication::setFont(defaultFont);
    }
    
    const QString appDir = QApplication::applicationDirPath();
    acss::QtAdvancedStylesheet styleManager;
    styleManager.setStylesDirPath(R"(:/styles)");
    styleManager.setOutputDirPath(appDir + "/StylesOutput");
    styleManager.setCurrentStyle("qt_material_modified");
    styleManager.setCurrentTheme("light_blue");
    // styleManager.updateStylesheet();
    // qApp->setStyleSheet(styleManager.styleSheet());
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

    if (serverIP.empty()) {
        serverIP = "127.0.0.1";
    }

    if (serverPort.empty()) {
        serverPort = "8088";
    }

    auto endPoint = serverIP + ":" + serverPort;

    SeeleInfoTag("main", "Server IP: {}", serverIP);
    SeeleInfoTag("main", "Server Port: {}", serverPort);

    RpcCall::getInstance().initialize(endPoint);

    // auto fu = asio::co_spawn(RpcCall::getInstance().GrpcContext(), []() -> asio::awaitable<grpc::Status> {
    //     co_return co_await WrappedCall::UserLoginAsync();
    // }, asio::use_future);
    //
    // std::cout<<"UserLoginAsync: "<<fu.get().ok()<<std::endl;

    if (LoginWindow loginWindow{}; loginWindow.exec() != QDialog::Accepted) {
        return -1;
    }

    MainWindow mainwindow{};
    mainwindow.resize(1920 / 2, 1080 / 2);
    mainwindow.show();

    auto exitCode = QApplication::exec();
    if (exitCode == RestartCode) {
        QProcess::startDetached(qApp->applicationFilePath(), QStringList());
    }

    Seele::Log::Shutdown();
}
