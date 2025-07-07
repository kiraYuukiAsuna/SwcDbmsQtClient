#pragma once
#include <Message/Message.pb.h>

class CachedProtoData {
public:
	static CachedProtoData& getInstance() {
		static CachedProtoData instance;
		return instance;
	}

	proto::UserMetaInfoV1 CachedUserMetaInfo;
	bool OnlineStatus = false;

	std::string UserName;
	std::string UserUuid;
	std::string UserToken;

private:
	CachedProtoData();
};
