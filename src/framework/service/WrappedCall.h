#pragma once

#include <QMessageBox>
#include <QString>
#include "grpcpp/client_context.h"
#include "Message/Request.pb.h"
#include "RpcCall.h"
#include "CachedProtoData.h"

class WrappedCall{
public:
    static bool getAllProjectMetaInfo(proto::GetAllProjectResponse& response, QWidget* parent){
        grpc::ClientContext context;
        proto::GetAllProjectRequest request;
        auto* userInfo = request.mutable_userinfo();
        userInfo->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);

        auto& rpcCall = RpcCall::getInstance();
        auto status = rpcCall.Stub()->GetAllProject(&context, request, &response);
        if(status.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","GetAllProjectMetaInfo Failed!" + QString::fromStdString(response.message()));
            }

        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(status.error_message()));
        }
        return false;
    }

    static bool getAllSwcMetaInfo(proto::GetAllSwcMetaInfoResponse& response, QWidget* parent){
        grpc::ClientContext context;
        proto::GetAllSwcMetaInfoRequest request;
        auto* userInfo = request.mutable_userinfo();
        userInfo->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);

        auto& rpcCall = RpcCall::getInstance();
        auto status = rpcCall.Stub()->GetAllSwcMetaInfo(&context, request, &response);
        if(status.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Error","GetAllSwcMetaInfo Failed!" + QString::fromStdString(response.message()));
            }

        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(status.error_message()));
        }
        return false;
    }

    static bool getAllDailyStatisticsMetaInfo(proto::GetAllDailyStatisticsResponse& response, QWidget* parent){
        grpc::ClientContext context;
        proto::GetAllDailyStatisticsRequest request;
        auto* userInfo = request.mutable_userinfo();
        userInfo->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);

        auto& rpcCall = RpcCall::getInstance();
        auto status = rpcCall.Stub()->GetAllDailyStatistics(&context, request, &response);
        if(status.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","GetAllDailyStatistics Failed!" + QString::fromStdString(response.message()));
            }

        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(status.error_message()));
        }
        return false;
    }

    static bool getAllUserMetaInfo(proto::GetAllUserResponse& response, QWidget* parent){
        grpc::ClientContext context;
        proto::GetAllUserRequest request;
        auto* userInfo = request.mutable_userinfo();
        userInfo->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);

        auto& rpcCall = RpcCall::getInstance();
        auto status = rpcCall.Stub()->GetAllUser(&context, request, &response);
        if(status.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","GetAllUserMetaInfo Failed!" + QString::fromStdString(response.message()));
            }

        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(status.error_message()));
        }
        return false;
    }

    static bool getProjectMetaInfoByName(const std::string& projectName, proto::GetProjectResponse& response, QWidget* parent) {
        proto::GetProjectRequest request;
        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
        request.mutable_projectinfo()->set_name(projectName);

        grpc::ClientContext context;
        auto result = RpcCall::getInstance().Stub()->GetProject(&context, request,&response);
        if(result.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","GetProjectMetaInfo Failed!" + QString::fromStdString(response.message()));
            }
        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool getSwcMetaInfoByName(const std::string& swcName, proto::GetSwcMetaInfoResponse& response, QWidget* parent){
        proto::GetSwcMetaInfoRequest request;
        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
        request.mutable_swcinfo()->set_name(swcName);


        grpc::ClientContext context;
        auto result = RpcCall::getInstance().Stub()->GetSwcMetaInfo(&context, request,&response);
        if(result.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","GetSwcMetaInfo Failed!" + QString::fromStdString(response.message()));
            }

        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool getDailyStatisticsmMetaInfoByName(const std::string& dailyStatisticsName, proto::GetDailyStatisticsResponse& response, QWidget* parent) {
        proto::GetDailyStatisticsRequest request;
        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
        request.mutable_dailystatisticsinfo()->set_name(dailyStatisticsName);

        grpc::ClientContext context;
        auto result = RpcCall::getInstance().Stub()->GetDailyStatistics(&context, request,&response);
        if(result.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","GetSwcMetaInfo Failed!" + QString::fromStdString(response.message()));
            }

        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool getSwcFullNodeData(const std::string& swcName, proto::GetSwcFullNodeDataResponse& response, QWidget* parent) {
        proto::GetSwcFullNodeDataRequest request;
        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
        request.mutable_swcinfo()->set_name(swcName);

        grpc::ClientContext context;
        auto result = RpcCall::getInstance().Stub()->GetSwcFullNodeData(&context, request,&response);
        if(result.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","GetSwcFullNodeData Failed!" + QString::fromStdString(response.message()));
            }

        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool getSwcNodeDataListByTimeAndUserResponse(const std::string& swcName, const std::string& userName, google::protobuf::Timestamp& startTime, google::protobuf::Timestamp& endTime, proto::GetSwcNodeDataListByTimeAndUserResponse& response, QWidget* parent) {
        proto::GetSwcNodeDataListByTimeAndUserRequest request;
        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
        request.mutable_swcinfo()->set_name(swcName);
        request.set_username(userName);
        request.mutable_starttime()->CopyFrom(startTime);
        request.mutable_endtime()->CopyFrom(endTime);

        grpc::ClientContext context;
        auto result = RpcCall::getInstance().Stub()->GetSwcNodeDataListByTimeAndUser(&context, request,&response);
        if(result.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","GetSwcNodeDataListByTimeAndUser Failed!" + QString::fromStdString(response.message()));
            }

        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool addSwcNodeData(const std::string& swcName, proto::SwcDataV1& swcData, proto::CreateSwcNodeDataResponse& response, QWidget* parent) {
        proto::CreateSwcNodeDataRequest request;
        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
        request.mutable_swcinfo()->set_name(swcName);
        request.mutable_swcdata()->CopyFrom(swcData);

        grpc::ClientContext context;
        auto result = RpcCall::getInstance().Stub()->CreateSwcNodeData(&context, request,&response);
        if(result.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","CreateSwcNodeData Failed!" + QString::fromStdString(response.message()));
            }
        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool modifySwcNodeData(const std::string& swcName, proto::SwcNodeDataV1& swcNodeData, proto::UpdateSwcNodeDataResponse & response, QWidget* parent) {
        proto::UpdateSwcNodeDataRequest request;
        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
        request.mutable_swcinfo()->set_name(swcName);
        request.mutable_swcnodedata()->CopyFrom(swcNodeData);

        grpc::ClientContext context;
        auto result = RpcCall::getInstance().Stub()->UpdateSwcNodeData(&context, request,&response);
        if(result.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","UpdateSwcNodeData Failed!" + QString::fromStdString(response.message()));
            }
        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool deleteSwcNodeData(const std::string& swcName, proto::SwcDataV1& swcData, proto::DeleteSwcNodeDataResponse& response, QWidget* parent) {
        proto::DeleteSwcNodeDataRequest request;
        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
        request.mutable_swcinfo()->set_name(swcName);
        request.mutable_swcdata()->CopyFrom(swcData);

        grpc::ClientContext context;
        auto result = RpcCall::getInstance().Stub()->DeleteSwcNodeData(&context, request,&response);
        if(result.ok()){
            if(response.status()) {
                return true;
            }else {
                QMessageBox::critical(parent,"Info","DeleteSwcNodeData Failed!" + QString::fromStdString(response.message()));
            }
        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool createSwcMeta(const std::string& name, const std::string& description , proto::CreateSwcResponse& response, QWidget* parent){
        proto::CreateSwcRequest request;
        grpc::ClientContext context;

        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);

        request.mutable_swcinfo()->set_name(name);
        request.mutable_swcinfo()->set_description(description);
        request.mutable_swcinfo()->set_swctype("eswc"); // by default when creating new swc using eswc type

        auto status = RpcCall::getInstance().Stub()->CreateSwc(&context, request, &response);
        if (status.ok()) {
            if (response.status()) {
                return true;
            }
            QMessageBox::critical(parent, "Error", QString::fromStdString(response.message()));
        }
        QMessageBox::critical(parent, "Error", QString::fromStdString(status.error_message()));
        return false;
    }

};
