#pragma once
#include <QObject>

enum class MetaInfoType {
	eProjectContainer = 0,
	eProject,
	eFreeSwcContainer,
	eProjectSwc,
	eFreeSwc,
	eDailyStatisticsContainer,
	eDailyStatistics,
	eUserMetaInfo,
	ePermissionGroupMetaInfo,
	eUserManagerMetaInfo,
	eSwcData,
	eSwcFeature,
	eQualityControl,
	eUnknown
};

struct LeftClientViewTreeWidgetItemMetaInfo {
	MetaInfoType type;
	std::string uuid;
	std::string name;
};
Q_DECLARE_METATYPE(LeftClientViewTreeWidgetItemMetaInfo)

inline constexpr int RestartCode = 773;
