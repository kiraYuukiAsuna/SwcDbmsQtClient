#pragma once

#include <fstream>
#include <QStandardPaths>
#include <string>
#include <QCoreApplication>
#include <iostream>
#include "json.hpp"

class AppConfig {
public:
    enum class SecurityConfigItem {
        eCachedUserName = 0,
        eCachedPassword,
        eAccountExpiredTime
    };

    enum class ConfigItem {
        eServerIP = 0,
        eServerPort
    };

    static AppConfig &getInstance() {
        static AppConfig instance;
        return instance;
    }

    void initialize(const std::string &appSecurityConfigFile, const std::string &appConfigFile) {
        m_AppSecurityConfigFile = appSecurityConfigFile;
        m_AppConfigFile = appConfigFile;


        std::filesystem::path path(m_AppRoamingPath);
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directories(path);
        }
    }

    void readConfig() {
        std::filesystem::path path(m_AppRunningPath);
        path = path / m_AppConfigFile;
        std::ifstream f(path.string());
        try {
            m_AppConfig = nlohmann::json::parse(f);
        } catch (...) {

        }
        f.close();
    }


    void readSecurityConfig() {
        std::filesystem::path path(m_AppRoamingPath);
        path = path / m_AppSecurityConfigFile;
        std::ifstream f(path.string());
        try {
            m_AppSecurityConfig = nlohmann::json::parse(f);
        } catch (...) {

        }
        f.close();
    }

    void writeSecurityConfig() {
        std::filesystem::path path(m_AppRoamingPath);
        path = path / m_AppSecurityConfigFile;
        std::ofstream f(path.string());
        f << m_AppSecurityConfig;
        f.close();
    }

    std::string getConfig(ConfigItem configItem) {
        switch (configItem) {
            case ConfigItem::eServerIP: {
                if (m_AppConfig.contains("eServerIP")) {
                    return m_AppConfig["eServerIP"];
                } else {
                    return "";
                }
            }
            case ConfigItem::eServerPort: {
                if (m_AppConfig.contains("eServerPort")) {
                    return m_AppConfig["eServerPort"];
                } else {
                    return "";
                }
            }
            default: {
                return "ConfigError";
            }
        }
    }

    std::string getSecurityConfig(SecurityConfigItem configItem) {
        switch (configItem) {
            case SecurityConfigItem::eCachedUserName: {
                if (m_AppSecurityConfig.contains("eCachedUserName")) {
                    return m_AppSecurityConfig["eCachedUserName"];
                } else {
                    return "";
                }
            }
            case SecurityConfigItem::eCachedPassword: {
                if (m_AppSecurityConfig.contains("eCachedPassword")) {
                    return m_AppSecurityConfig["eCachedPassword"];
                } else {
                    return "";
                }
            }
            case SecurityConfigItem::eAccountExpiredTime: {
                if (m_AppSecurityConfig.contains("eAccountExpiredTime")) {
                    return m_AppSecurityConfig["eAccountExpiredTime"];
                } else {
                    return "";
                }
            }
            default: {
                return "ConfigError";
            }
        }
    }

    void setSecurityConfig(SecurityConfigItem configItem, const std::string &configData) {
        switch (configItem) {
            case SecurityConfigItem::eCachedUserName: {
                m_AppSecurityConfig["eCachedUserName"] = configData;
                break;
            }
            case SecurityConfigItem::eCachedPassword: {
                m_AppSecurityConfig["eCachedPassword"] = configData;
                break;
            }
            case SecurityConfigItem::eAccountExpiredTime: {
                m_AppSecurityConfig["eAccountExpiredTime"] = configData;
                break;
            }
            default: {
                break;
            }
        }
    }

private:
    AppConfig() {
        m_AppRoamingPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString();
        m_AppRunningPath = QCoreApplication::applicationDirPath().toStdString();
    }

    std::string m_AppRoamingPath;
    std::string m_AppRunningPath;

    std::string m_AppSecurityConfigFile;
    std::string m_AppConfigFile;

    nlohmann::json m_AppSecurityConfig;
    nlohmann::json m_AppConfig;
};

