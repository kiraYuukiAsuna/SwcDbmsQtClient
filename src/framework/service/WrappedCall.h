#pragma once

#include <QMessageBox>
#include <QString>
#include <grpcpp/client_context.h>
#include "Message/Request.pb.h"
#include "RpcCall.h"
#include "CachedProtoData.h"

class WrappedCall {
public:
    template<typename T>
    static void setCommonRequestField(T&type) {
        type.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
        auto* userInfo = type.mutable_userverifyinfo();
        userInfo->set_username(CachedProtoData::getInstance().UserName);
        userInfo->set_usertoken(CachedProtoData::getInstance().UserToken);
    }

    static bool defaultErrorHandler(const grpc::Status&status, const proto::ResponseMetaInfoV1&rspMeta,
                                    QWidget* parent) {
        if (status.ok()) {
            if (rspMeta.status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "GetAllProjectMetaInfo Failed!" + QString::fromStdString(
                                      rspMeta.message()));
        }
        QMessageBox::critical(parent, "Error", QString::fromStdString(status.error_message()));
        return false;
    }

    static bool getAllProjectMetaInfo(proto::GetAllProjectResponse&response, QWidget* parent) {
        proto::GetAllProjectRequest request;
        setCommonRequestField(request);

        grpc::ClientContext context;
        if (auto status = RpcCall::getInstance().Stub()->GetAllProject(&context, request, &response); status.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "GetAllProjectMetaInfo Failed!" + QString::fromStdString(
                                      response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(status.error_message()));
        }
        return false;
    }

    static bool getAllSwcMetaInfo(proto::GetAllSwcMetaInfoResponse&response, QWidget* parent) {
        proto::GetAllSwcMetaInfoRequest request;
        setCommonRequestField(request);

        grpc::ClientContext context;
        if (auto status = RpcCall::getInstance().Stub()->GetAllSwcMetaInfo(&context, request, &response); status.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Error",
                                  "GetAllSwcMetaInfo Failed!" + QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(status.error_message()));
        }
        return false;
    }

    static bool getAllDailyStatisticsMetaInfo(proto::GetAllDailyStatisticsResponse&response, QWidget* parent) {
        proto::GetAllDailyStatisticsRequest request;
        setCommonRequestField(request);

        grpc::ClientContext context;
        if (auto status = RpcCall::getInstance().Stub()->GetAllDailyStatistics(&context, request, &response); status.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "GetAllDailyStatistics Failed!" + QString::fromStdString(
                                      response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(status.error_message()));
        }
        return false;
    }

    static bool getAllUserMetaInfo(proto::GetAllUserResponse&response, QWidget* parent) {
        proto::GetAllUserRequest request;
        setCommonRequestField(request);

        grpc::ClientContext context;
        if (auto status = RpcCall::getInstance().Stub()->GetAllUser(&context, request, &response); status.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "GetAllUserMetaInfo Failed!" + QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(status.error_message()));
        }
        return false;
    }

    static bool getProjectMetaInfoByName(const std::string&projectName, proto::GetProjectResponse&response,
                                         QWidget* parent) {
        proto::GetProjectRequest request;
        setCommonRequestField(request);
        request.set_projectname(projectName);

        grpc::ClientContext context;
        if (auto result = RpcCall::getInstance().Stub()->GetProject(&context, request, &response); result.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "GetProjectMetaInfo Failed!" + QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool getSwcMetaInfoByName(const std::string&swcName, proto::GetSwcMetaInfoResponse&response,
                                     QWidget* parent) {
        proto::GetSwcMetaInfoRequest request;
        setCommonRequestField(request);
        request.set_swcname(swcName);

        grpc::ClientContext context;
        if (auto result = RpcCall::getInstance().Stub()->GetSwcMetaInfo(&context, request, &response); result.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "GetSwcMetaInfo Failed!" + QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool getDailyStatisticsmMetaInfoByName(const std::string&dailyStatisticsName,
                                                  proto::GetDailyStatisticsResponse&response, QWidget* parent) {
        proto::GetDailyStatisticsRequest request;
        setCommonRequestField(request);
        request.set_dailystatisticsname(dailyStatisticsName);

        grpc::ClientContext context;
        if (auto result = RpcCall::getInstance().Stub()->GetDailyStatistics(&context, request, &response); result.
            ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "GetSwcMetaInfo Failed!" + QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool getSwcFullNodeData(const std::string&swcName, proto::GetSwcFullNodeDataResponse&response,
                                   QWidget* parent) {
        proto::GetSwcFullNodeDataRequest request;
        setCommonRequestField(request);
        request.set_swcname(swcName);

        grpc::ClientContext context;
        if (auto result = RpcCall::getInstance().Stub()->GetSwcFullNodeData(&context, request, &response); result.
            ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "GetSwcFullNodeData Failed!" + QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool getSwcSnapshot(const std::string&swcSnapshotCollectioNname, proto::GetSnapshotResponse&response,
                               QWidget* parent) {
        proto::GetSnapshotRequest request;
        setCommonRequestField(request);
        request.set_swcsnapshotcollectionname(swcSnapshotCollectioNname);

        grpc::ClientContext context;
        if (auto result = RpcCall::getInstance().Stub()->GetSnapshot(&context, request, &response); result.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "GetSnapshot Failed!" + QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool getSwcNodeDataListByTimeAndUserResponse(const std::string&swcName, const std::string&userName,
                                                        google::protobuf::Timestamp&startTime,
                                                        google::protobuf::Timestamp&endTime,
                                                        proto::GetSwcNodeDataListByTimeAndUserResponse&response,
                                                        QWidget* parent) {
        proto::GetSwcNodeDataListByTimeAndUserRequest request;
        setCommonRequestField(request);
        request.set_swcname(swcName);
        request.set_username(userName);
        request.mutable_starttime()->CopyFrom(startTime);
        request.mutable_endtime()->CopyFrom(endTime);

        grpc::ClientContext context;
        if (auto result = RpcCall::getInstance().Stub()->GetSwcNodeDataListByTimeAndUser(&context, request, &response);
            result.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "GetSwcNodeDataListByTimeAndUser Failed!" + QString::fromStdString(
                                      response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool addSwcNodeData(const std::string&swcName, proto::SwcDataV1&swcData,
                               proto::CreateSwcNodeDataResponse&response, QWidget* parent) {
        proto::CreateSwcNodeDataRequest request;
        setCommonRequestField(request);
        request.set_swcname(swcName);
        request.mutable_swcdata()->CopyFrom(swcData);

        grpc::ClientContext context;
        if (auto result = RpcCall::getInstance().Stub()->CreateSwcNodeData(&context, request, &response); result.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "CreateSwcNodeData Failed!" + QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool modifySwcNodeData(const std::string&swcName, proto::SwcNodeDataV1&swcNodeData,
                                  proto::UpdateSwcNodeDataResponse&response, QWidget* parent) {
        proto::UpdateSwcNodeDataRequest request;
        setCommonRequestField(request);
        request.set_swcname(swcName);
        request.mutable_swcnodedata()->CopyFrom(swcNodeData);

        grpc::ClientContext context;
        if (auto result = RpcCall::getInstance().Stub()->UpdateSwcNodeData(&context, request, &response); result.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "UpdateSwcNodeData Failed!" + QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool deleteSwcNodeData(const std::string&swcName, proto::SwcDataV1&swcData,
                                  proto::DeleteSwcNodeDataResponse&response, QWidget* parent) {
        proto::DeleteSwcNodeDataRequest request;
        setCommonRequestField(request);
        request.set_swcname(swcName);
        request.mutable_swcdata()->CopyFrom(swcData);

        grpc::ClientContext context;
        if (auto result = RpcCall::getInstance().Stub()->DeleteSwcNodeData(&context, request, &response); result.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Info",
                                  "DeleteSwcNodeData Failed!" + QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(result.error_message()));
        }
        return false;
    }

    static bool createSwcMeta(const std::string&name, const std::string&description, proto::CreateSwcResponse&response,
                              QWidget* parent) {
        proto::CreateSwcRequest request;
        setCommonRequestField(request);
        request.mutable_swcinfo()->set_name(name);
        request.mutable_swcinfo()->set_description(description);
        request.mutable_swcinfo()->set_swctype("eswc"); // by default when creating new swc using eswc type

        grpc::ClientContext context;
        if (auto status = RpcCall::getInstance().Stub()->CreateSwc(&context, request, &response); status.ok()) {
            if (response.metainfo().status()) {
                return true;
            }
            QMessageBox::critical(parent, "Error", QString::fromStdString(response.metainfo().message()));
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(status.error_message()));
        }
        return false;
    }

    static bool getAllSwcIncrementRecord(const std::string&name,
                                         proto::GetAllIncrementOperationMetaInfoResponse&response,
                                         QWidget* parent) {
        proto::GetAllIncrementOperationMetaInfoRequest request;
        setCommonRequestField(request);
        request.set_swcname(name);

        grpc::ClientContext context;
        if (auto status = RpcCall::getInstance().Stub()->GetAllIncrementOperationMetaInfo(&context, request, &response);
            status.ok()) {
            if (response.has_metainfo() && response.metainfo().status() == true) {
                return true;
            }
            QMessageBox::critical(parent, "Error", QString::fromStdString(response.metainfo().message()));
            return false;
        }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(status.error_message()));
            return false;
        }
    }

    static bool getSwcIncrementRecord(const std::string&name,
                                     proto::GetIncrementOperationResponse&response,
                                     QWidget* parent) {
        proto::GetIncrementOperationRequest request;
        setCommonRequestField(request);
        request.set_incrementoperationcollectionname(name);

        grpc::ClientContext context;
        if (auto status = RpcCall::getInstance().Stub()->GetIncrementOperation(&context, request, &response);
            status.ok()) {
            if (response.has_metainfo() && response.metainfo().status() == true) {
                return true;
            }
            QMessageBox::critical(parent, "Error", QString::fromStdString(response.metainfo().message()));
            return false;
            }
        else {
            QMessageBox::critical(parent, "Error", QString::fromStdString(status.error_message()));
            return false;
        }
    }


};
