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

private:
    CachedProtoData();
};
