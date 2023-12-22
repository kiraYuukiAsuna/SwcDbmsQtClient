#pragma once


#include <QWidget>
#include <Message/Response.pb.h>

#include "EditorBase.h"


QT_BEGIN_NAMESPACE
namespace Ui { class EditorDailyStatisticsMetaInfo; }
QT_END_NAMESPACE

class EditorDailyStatisticsMetaInfo : public QWidget, public EditorBase{
Q_OBJECT

public:
    explicit EditorDailyStatisticsMetaInfo(proto::GetDailyStatisticsResponse& response, QWidget *parent = nullptr);
    ~EditorDailyStatisticsMetaInfo() override;

    std::string getName() override {
        return m_DailyStatisticsMetaInfo.name();
    }

    MetaInfoType getMetaInfoType() override {
        return MetaInfoType::eDailyStatistics;
    }

    void refresh(proto::GetDailyStatisticsResponse& response);

    virtual bool save();

private:
    Ui::EditorDailyStatisticsMetaInfo *ui;
    proto::DailyStatisticsMetaInfoV1 m_DailyStatisticsMetaInfo;
};

