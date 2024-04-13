#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <google/protobuf/timestamp.pb.h>

inline std::vector<std::string> stringSplit(const std::string&str, char delim) {
    std::stringstream ss(str);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

inline bool isEarlier(const google::protobuf::Timestamp& t1, const google::protobuf::Timestamp& t2) {
    if (t1.seconds() < t2.seconds()) {
        return true;
    } else if (t1.seconds() > t2.seconds()) {
        return false;
    } else {
        return t1.nanos() < t2.nanos();
    }
}
