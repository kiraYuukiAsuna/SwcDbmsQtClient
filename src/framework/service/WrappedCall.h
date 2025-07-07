#pragma once

#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>

#include <QMessageBox>
#include <QString>
#include <agrpc/asio_grpc.hpp>
#include <asio.hpp>

#include "CachedProtoData.h"
#include "Message/Request.pb.h"
#include "RpcCall.h"
#include "Service/Service.grpc.pb.h"
#include "src/framework/core/log/Log.h"

template <typename T>
struct RPCTraitsAbstract;

template <typename StubT, typename RequestT, typename ResponseT,
		  std::unique_ptr<grpc::ClientAsyncResponseReader<ResponseT>> (
			  StubT::*PrepareAsyncUnary)(grpc::ClientContext*, const RequestT&,
										 grpc::CompletionQueue*),
		  typename Executor>
struct RPCTraitsAbstract<agrpc::ClientRPC<PrepareAsyncUnary, Executor>> {
	using Request = RequestT;
	using Response = ResponseT;
	using RequestClient = asio::use_awaitable_t<>::as_default_on_t<
		agrpc::ClientRPC<PrepareAsyncUnary, Executor>>;
};

template <auto A,
		  typename B =
			  asio::use_awaitable_t<>::as_default_on_t<agrpc::ClientRPC<A>>>
struct RPCTraits {
	using RequestType = typename RPCTraitsAbstract<B>::Request;
	using ResponseType = typename RPCTraitsAbstract<B>::Response;
	using RequestClient = B;

	RequestType request;
	ResponseType response;
};

template <typename RPCTraits>
static asio::awaitable<grpc::Status> commonReqRsp(
	std::shared_ptr<agrpc::GrpcContext> grpc_context, proto::DBMS::Stub& stub,
	RPCTraits& rpc) {
	grpc::ClientContext clientContext;
	clientContext.set_deadline(std::chrono::system_clock::now() +
							   std::chrono::seconds(5));
	grpc::Status status = co_await RPCTraits::RequestClient::request(
		*grpc_context, stub, clientContext, rpc.request, rpc.response);
	co_return status;
}

class WrappedCall {
public:
	template <typename T>
		requires std::is_base_of_v<google::protobuf::Message, T>
	static void setCommonRequestField(T& type) {
		type.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
		auto* userInfo = type.mutable_userverifyinfo();
		userInfo->set_username(CachedProtoData::getInstance().UserName);
		userInfo->set_usertoken(CachedProtoData::getInstance().UserToken);
	}

	static asio::awaitable<grpc::Status> UserLoginAsync() {
		RPCTraits<&proto::DBMS::Stub::PrepareAsyncUserLogin> rpc;
		rpc.request.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
		rpc.request.set_username("Hanasaka");
		rpc.request.set_password("Hanasaka2");
		auto status =
			co_await commonReqRsp(RpcCall::getInstance().GrpcContext(),
								  *RpcCall::getInstance().Stub(), rpc);
		co_return status;
	}

	static bool defaultErrorHandler(const std::string& actionName,
									const grpc::Status& status,
									const proto::ResponseMetaInfoV1& rspMeta,
									QWidget* parent) {
		if (status.ok()) {
			if (rspMeta.status()) {
				return true;
			}
			QMessageBox::critical(
				parent, "Error",
				QString::fromStdString(actionName + " Failed! " +
									   rspMeta.message()));
			return false;
		}
		QMessageBox::critical(parent, "Error",
							  QString::fromStdString(status.error_message()));
		return false;
	}

	static bool defaultErrorHandlerNoMessageBox(
		const std::string& actionName, const grpc::Status& status,
		const proto::ResponseMetaInfoV1& rspMeta, QWidget* parent) {
		if (status.ok()) {
			if (rspMeta.status()) {
				return true;
			}
			SeeleErrorTag("WrappedCall", "{}",
						  actionName + " Failed! " + rspMeta.message());
			return false;
		}
		SeeleErrorTag("WrappedCall", "{}",
					  actionName + " Failed! " + status.error_message());
		return false;
	}

	static bool getAllProjectMetaInfo(proto::GetAllProjectResponse& response,
									  QWidget* parent) {
		proto::GetAllProjectRequest request;
		setCommonRequestField(request);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetAllProject(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool getAllSwcMetaInfo(proto::GetAllSwcMetaInfoResponse& response,
								  QWidget* parent) {
		proto::GetAllSwcMetaInfoRequest request;
		setCommonRequestField(request);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetAllSwcMetaInfo(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool getAllDailyStatisticsMetaInfo(
		proto::GetAllDailyStatisticsResponse& response, QWidget* parent) {
		proto::GetAllDailyStatisticsRequest request;
		setCommonRequestField(request);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetAllDailyStatistics(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool getAllUserMetaInfo(proto::GetAllUserResponse& response,
								   QWidget* parent) {
		proto::GetAllUserRequest request;
		setCommonRequestField(request);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetAllUser(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool getProjectMetaInfoByUuid(const std::string& projectUuid,
										 proto::GetProjectResponse& response,
										 QWidget* parent) {
		proto::GetProjectRequest request;
		setCommonRequestField(request);
		request.set_projectuuid(projectUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetProject(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool getSwcMetaInfoByUuid(const std::string& swcUuid,
									 proto::GetSwcMetaInfoResponse& response,
									 QWidget* parent) {
		proto::GetSwcMetaInfoRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetSwcMetaInfo(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool getDailyStatisticsmMetaInfoByName(
		const std::string& dailyStatisticsName,
		proto::GetDailyStatisticsResponse& response, QWidget* parent) {
		proto::GetDailyStatisticsRequest request;
		setCommonRequestField(request);
		request.set_dailystatisticsname(dailyStatisticsName);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetDailyStatistics(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool getSwcFullNodeDataByUuid(
		const std::string& swcUuid, proto::GetSwcFullNodeDataResponse& response,
		QWidget* parent, bool noMessageBoxWhenError = false) {
		proto::GetSwcFullNodeDataRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetSwcFullNodeData(
			&context, request, &response);
		if (noMessageBoxWhenError) {
			return defaultErrorHandlerNoMessageBox(__func__, status,
												   response.metainfo(), parent);
		} else {
			return defaultErrorHandler(__func__, status, response.metainfo(),
									   parent);
		}
	}

	static bool getSwcSnapshot(const std::string& swcSnapshotCollectioNname,
							   proto::GetSnapshotResponse& response,
							   QWidget* parent,
							   bool noMessageBoxWhenError = false) {
		proto::GetSnapshotRequest request;
		setCommonRequestField(request);
		request.set_swcsnapshotcollectionname(swcSnapshotCollectioNname);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetSnapshot(
			&context, request, &response);
		if (noMessageBoxWhenError) {
			return defaultErrorHandlerNoMessageBox(__func__, status,
												   response.metainfo(), parent);
		} else {
			return defaultErrorHandler(__func__, status, response.metainfo(),
									   parent);
		}
	}

	static bool getSwcNodeDataListByTimeAndUserByUuid(
		const std::string& swcUuid, const std::string& userName,
		google::protobuf::Timestamp& startTime,
		google::protobuf::Timestamp& endTime,
		proto::GetSwcNodeDataListByTimeAndUserResponse& response,
		QWidget* parent) {
		proto::GetSwcNodeDataListByTimeAndUserRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.set_username(userName);
		request.mutable_starttime()->CopyFrom(startTime);
		request.mutable_endtime()->CopyFrom(endTime);

		grpc::ClientContext context;
		auto status =
			RpcCall::getInstance().Stub()->GetSwcNodeDataListByTimeAndUser(
				&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool addSwcNodeDataByUuid(const std::string& swcUuid,
									 proto::SwcDataV1& swcData,
									 proto::CreateSwcNodeDataResponse& response,
									 QWidget* parent,
									 bool noMessageBoxWhenError = false) {
		proto::CreateSwcNodeDataRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.mutable_swcdata()->CopyFrom(swcData);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->CreateSwcNodeData(
			&context, request, &response);
		if (noMessageBoxWhenError) {
			return defaultErrorHandlerNoMessageBox(__func__, status,
												   response.metainfo(), parent);
		} else {
			return defaultErrorHandler(__func__, status, response.metainfo(),
									   parent);
		}
	}

	static bool modifySwcNodeDataByUuid(
		const std::string& swcUuid, proto::SwcDataV1& swcData,
		proto::UpdateSwcNodeDataResponse& response, QWidget* parent) {
		proto::UpdateSwcNodeDataRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.mutable_swcdata()->CopyFrom(swcData);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->UpdateSwcNodeData(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool deleteSwcNodeDataByUuid(
		const std::string& swcUuid, proto::SwcDataV1& swcData,
		proto::DeleteSwcNodeDataResponse& response, QWidget* parent) {
		proto::DeleteSwcNodeDataRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.mutable_swcdata()->CopyFrom(swcData);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->DeleteSwcNodeData(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool createSwcMeta(const std::string& name,
							  const std::string& description,
							  std::string belongToProjectUuid,
							  proto::CreateSwcResponse& response,
							  QWidget* parent,
							  bool noMessageBoxWhenError = false) {
		proto::CreateSwcRequest request;
		setCommonRequestField(request);
		request.mutable_swcinfo()->set_name(name);
		request.mutable_swcinfo()->set_description(description);
		request.mutable_swcinfo()->set_swctype(
			"eswc");  // by default when creating new swc using eswc type
		request.mutable_swcinfo()->set_belongingprojectuuid(
			belongToProjectUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->CreateSwc(
			&context, request, &response);
		if (noMessageBoxWhenError) {
			return defaultErrorHandlerNoMessageBox(__func__, status,
												   response.metainfo(), parent);
		} else {
			return defaultErrorHandler(__func__, status, response.metainfo(),
									   parent);
		}
	}

	static bool getAllSwcIncrementRecordByUuid(
		const std::string& swcUuid,
		proto::GetAllIncrementOperationMetaInfoResponse& response,
		QWidget* parent) {
		proto::GetAllIncrementOperationMetaInfoRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);

		grpc::ClientContext context;
		auto status =
			RpcCall::getInstance().Stub()->GetAllIncrementOperationMetaInfo(
				&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool getSwcIncrementRecord(
		const std::string& name, proto::GetIncrementOperationResponse& response,
		QWidget* parent) {
		proto::GetIncrementOperationRequest request;
		setCommonRequestField(request);
		request.set_incrementoperationcollectionname(name);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetIncrementOperation(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool getSwcAttachmentAnoByUuid(
		const std::string& swcUuid, const std::string& attachmentUuid,
		proto::GetSwcAttachmentAnoResponse& response, QWidget* parent,
		bool noMessageBoxWhenError = false) {
		proto::GetSwcAttachmentAnoRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.set_anoattachmentuuid(attachmentUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetSwcAttachmentAno(
			&context, request, &response);
		if (noMessageBoxWhenError) {
			return defaultErrorHandlerNoMessageBox(__func__, status,
												   response.metainfo(), parent);
		} else {
			return defaultErrorHandler(__func__, status, response.metainfo(),
									   parent);
		}
	}

	static bool createSwcAttachmentAno(
		const std::string& swcUuid, const std::string& apoFileName,
		const std::string& swcFileName,
		proto::CreateSwcAttachmentAnoResponse& response, QWidget* parent,
		bool noMessageBoxWhenError = false) {
		proto::CreateSwcAttachmentAnoRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.mutable_swcattachmentano()->set_apofile(apoFileName);
		request.mutable_swcattachmentano()->set_swcfile(swcFileName);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->CreateSwcAttachmentAno(
			&context, request, &response);
		if (noMessageBoxWhenError) {
			return defaultErrorHandlerNoMessageBox(__func__, status,
												   response.metainfo(), parent);
		} else {
			return defaultErrorHandler(__func__, status, response.metainfo(),
									   parent);
		}
	}

	static bool updateSwcAttachmentAnoByUuid(
		const std::string& swcUuid, const std::string& attachmentUuid,
		const std::string& apoFileName, const std::string& swcFileName,
		proto::UpdateSwcAttachmentAnoResponse& response, QWidget* parent) {
		proto::UpdateSwcAttachmentAnoRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.set_anoattachmentuuid(attachmentUuid);
		request.mutable_newswcattachmentano()->set_apofile(apoFileName);
		request.mutable_newswcattachmentano()->set_swcfile(swcFileName);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->UpdateSwcAttachmentAno(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool deleteSwcAttachmentAnoByUuid(
		const std::string& swcUuid, const std::string& attachmentUuid,
		proto::DeleteSwcAttachmentAnoResponse& response, QWidget* parent) {
		proto::DeleteSwcAttachmentAnoRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.set_anoattachmentuuid(attachmentUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->DeleteSwcAttachmentAno(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool getSwcAttachmentApoByUuid(
		const std::string& swcUuid, const std::string& attachmentUuid,
		proto::GetSwcAttachmentApoResponse& response, QWidget* parent,
		bool noMessageBoxWhenError = false) {
		proto::GetSwcAttachmentApoRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.set_apoattachmentuuid(attachmentUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetSwcAttachmentApo(
			&context, request, &response);
		if (noMessageBoxWhenError) {
			return defaultErrorHandlerNoMessageBox(__func__, status,
												   response.metainfo(), parent);
		} else {
			return defaultErrorHandler(__func__, status, response.metainfo(),
									   parent);
		}
	}

	static bool createSwcAttachmentApo(
		const std::string& swcUuid,
		std::vector<proto::SwcAttachmentApoV1> attachments,
		proto::CreateSwcAttachmentApoResponse& response, QWidget* parent,
		bool noMessageBoxWhenError = false) {
		proto::CreateSwcAttachmentApoRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		for (auto& attachment : attachments) {
			request.add_swcattachmentapo()->CopyFrom(attachment);
		}

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->CreateSwcAttachmentApo(
			&context, request, &response);
		if (noMessageBoxWhenError) {
			return defaultErrorHandlerNoMessageBox(__func__, status,
												   response.metainfo(), parent);
		} else {
			return defaultErrorHandler(__func__, status, response.metainfo(),
									   parent);
		}
	}

	static bool updateSwcAttachmentApoByUuid(
		const std::string& swcUuid, const std::string& attachmentUuid,
		std::vector<proto::SwcAttachmentApoV1> attachments,
		proto::UpdateSwcAttachmentApoResponse& response, QWidget* parent) {
		proto::UpdateSwcAttachmentApoRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.set_apoattachmentuuid(attachmentUuid);
		for (auto& attachment : attachments) {
			request.add_newswcattachmentapo()->CopyFrom(attachment);
		}

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->UpdateSwcAttachmentApo(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool deleteSwcAttachmentApoByUuid(
		const std::string& swcUuid, const std::string& attachmentUuid,
		proto::DeleteSwcAttachmentApoResponse& response, QWidget* parent) {
		proto::DeleteSwcAttachmentApoRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.set_apoattachmentuuid(attachmentUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->DeleteSwcAttachmentApo(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool RevertSwcVersionByUuid(
		const std::string& swcUuid, google::protobuf::Timestamp& endTime,
		proto::RevertSwcVersionResponse& response, QWidget* parent) {
		proto::RevertSwcVersionRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.mutable_versionendtime()->CopyFrom(endTime);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->RevertSwcVersion(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool GetPermissionGroupByUuid(
		const std::string& uuid,
		proto::GetPermissionGroupByUuidResponse& response, QWidget* parent,
		bool noMessageBoxWhenError = false) {
		proto::GetPermissionGroupByUuidRequest request;
		setCommonRequestField(request);
		request.set_permissiongroupuuid(uuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetPermissionGroupByUuid(
			&context, request, &response);
		if (noMessageBoxWhenError) {
			return defaultErrorHandlerNoMessageBox(__func__, status,
												   response.metainfo(), parent);
		}
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool GetPermissionGroupByName(
		const std::string& name,
		proto::GetPermissionGroupByNameResponse& response, QWidget* parent,
		bool noMessageBoxWhenError = false) {
		proto::GetPermissionGroupByNameRequest request;
		setCommonRequestField(request);
		request.set_permissiongroupname(name);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetPermissionGroupByName(
			&context, request, &response);
		if (noMessageBoxWhenError) {
			return defaultErrorHandlerNoMessageBox(__func__, status,
												   response.metainfo(), parent);
		}
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool GetAllPermissionGroup(
		proto::GetAllPermissionGroupResponse& response, QWidget* parent) {
		proto::GetAllPermissionGroupRequest request;
		setCommonRequestField(request);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetAllPermissionGroup(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool CreatePermissionGroup(
		const std::string& permissionGroupName,
		const std::string& permissionGroupDescription,
		proto::CreatePermissionGroupResponse& response, QWidget* parent) {
		proto::CreatePermissionGroupRequest request;
		setCommonRequestField(request);

		request.set_permissiongroupname(permissionGroupName);
		request.set_permissiongroupdescription(permissionGroupDescription);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->CreatePermissionGroup(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool DeletePermissionGroup(
		const std::string& permissionGroupUuid,
		proto::DeletePermissionGroupResponse& response, QWidget* parent) {
		proto::DeletePermissionGroupRequest request;
		setCommonRequestField(request);

		request.set_permissiongroupuuid(permissionGroupUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->DeletePermissionGroup(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool UpdatePermissionGroup(
		const std::string& permissionGroupUuid,
		const std::string& permissionGroupName,
		const std::string& permissionGroupDescription,
		proto::PermissionGroupAceV1 ace,
		proto::UpdatePermissionGroupResponse& response, QWidget* parent) {
		proto::UpdatePermissionGroupRequest request;
		setCommonRequestField(request);

		request.set_permissiongroupuuid(permissionGroupUuid);
		request.set_permissiongroupname(permissionGroupName);
		request.set_permissiongroupdescription(permissionGroupDescription);
		request.mutable_ace()->CopyFrom(ace);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->UpdatePermissionGroup(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool GetUserInfoByUuid(const std::string& userUuid,
								  proto::GetUserByUuidResponse& response,
								  QWidget* parent) {
		proto::GetUserByUuidRequest request;
		setCommonRequestField(request);

		request.set_useruuid(userUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetUserByUuid(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool GetUserInfoByName(const std::string& userName,
								  proto::GetUserByNameResponse& response,
								  QWidget* parent) {
		proto::GetUserByNameRequest request;
		setCommonRequestField(request);

		request.set_username(userName);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetUserByName(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool CreateUser(const std::string& userName,
						   const std::string& userPassword,
						   const std::string& userDescription,
						   proto::CreateUserResponse& response,
						   QWidget* parent) {
		proto::CreateUserRequest request;
		request.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
		request.mutable_userinfo()->set_name(userName);
		request.mutable_userinfo()->set_password(userPassword);
		request.mutable_userinfo()->set_description(userDescription);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->CreateUser(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool UpdateUser(proto::UserMetaInfoV1 userMetaInfo,
						   proto::UpdateUserResponse& response,
						   QWidget* parent) {
		proto::UpdateUserRequest request;
		setCommonRequestField(request);

		request.mutable_userinfo()->CopyFrom(userMetaInfo);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->UpdateUser(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool DeleteUser(const std::string userName,
						   proto::DeleteUserResponse& response,
						   QWidget* parent) {
		proto::DeleteUserRequest request;
		setCommonRequestField(request);

		request.set_username(userName);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->DeleteUser(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool UpdateSwcMetaInfo(const proto::SwcMetaInfoV1& metaInfo,
								  proto::UpdateSwcResponse& response,
								  QWidget* parent) {
		proto::UpdateSwcRequest request;
		setCommonRequestField(request);

		request.mutable_swcinfo()->CopyFrom(metaInfo);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->UpdateSwc(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool UpdateProjectMetaInfo(const proto::ProjectMetaInfoV1& metaInfo,
									  proto::UpdateProjectResponse& response,
									  QWidget* parent) {
		proto::UpdateProjectRequest request;
		setCommonRequestField(request);

		request.mutable_projectinfo()->CopyFrom(metaInfo);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->UpdateProject(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool GetSwcAttachmentSwc(
		const std::string& swcUuid, const std::string& attachmentUuid,
		proto::GetSwcAttachmentSwcResponse& response, QWidget* parent) {
		proto::GetSwcAttachmentSwcRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.set_swcattachmentuuid(attachmentUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetSwcAttachmentSwc(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool CreateSwcAttachmentSwcByUuid(
		const std::string& swcUuid,
		std::vector<proto::SwcNodeDataV1> attachments,
		proto::CreateSwcAttachmentSwcResponse& response, QWidget* parent) {
		proto::CreateSwcAttachmentSwcRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		for (auto& attachment : attachments) {
			request.add_swcdata()->CopyFrom(attachment);
		}

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->CreateSwcAttachmentSwc(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool UpdateSwcAttachmentSwcByUuid(
		const std::string& swcUuid, const std::string& attachmentUuid,
		std::vector<proto::SwcNodeDataV1> attachments,
		proto::UpdateSwcAttachmentSwcResponse& response, QWidget* parent) {
		proto::UpdateSwcAttachmentSwcRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.set_swcattachmentuuid(attachmentUuid);
		for (auto& attachment : attachments) {
			request.add_newswcdata()->CopyFrom(attachment);
		}

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->UpdateSwcAttachmentSwc(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool DeleteSwcAttachmentSwcByUuid(
		const std::string& swcUuid, const std::string& attachmentUuid,
		proto::DeleteSwcAttachmentSwcResponse& response, QWidget* parent) {
		proto::DeleteSwcAttachmentSwcRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);
		request.set_swcattachmentuuid(attachmentUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->DeleteSwcAttachmentSwc(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool GetProjectSwcNamesByProjectUuid(
		const std::string& projectUuid,
		proto::GetProjectSwcNamesByProjectUuidResponse& response,
		QWidget* parent) {
		proto::GetProjectSwcNamesByProjectUuidRequest request;
		setCommonRequestField(request);
		request.set_projectuuid(projectUuid);

		grpc::ClientContext context;
		auto status =
			RpcCall::getInstance().Stub()->GetProjectSwcNamesByProjectUuid(
				&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool GetAllProject(proto::GetAllProjectResponse& response,
							  QWidget* parent) {
		proto::GetAllProjectRequest request;
		setCommonRequestField(request);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->GetAllProject(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}

	static bool DeleteSwc(const std::string& swcUuid,
						  proto::DeleteSwcResponse& response, QWidget* parent) {
		proto::DeleteSwcRequest request;
		setCommonRequestField(request);
		request.set_swcuuid(swcUuid);

		grpc::ClientContext context;
		auto status = RpcCall::getInstance().Stub()->DeleteSwc(
			&context, request, &response);
		return defaultErrorHandler(__func__, status, response.metainfo(),
								   parent);
	}
};
