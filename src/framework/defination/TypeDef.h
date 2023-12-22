#pragma once
#include <QObject>

enum class MetaInfoType{
    eProjectContainer = 0,
    eProject,
    eSwcContainer,
    eSwc,
    eDailyStatisticsContainer,
    eDailyStatistics,
    eUserMetaInfo,
    ePermissionGroupMetaInfo,
    eUserManagerMetaInfo,
    eSwcData,
    eUnknown
};

struct LeftClientViewTreeWidgetItemMetaInfo{
    MetaInfoType type;
    std::string name;
};
Q_DECLARE_METATYPE(LeftClientViewTreeWidgetItemMetaInfo)

inline constexpr int RestartCode = 773;

