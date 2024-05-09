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

inline bool isEarlier(const google::protobuf::Timestamp&t1, const google::protobuf::Timestamp&t2) {
    if (t1.seconds() < t2.seconds()) {
        return true;
    }
    else if (t1.seconds() > t2.seconds()) {
        return false;
    }
    else {
        return t1.nanos() < t2.nanos();
    }
}

// Function to convert uint64 timestamp to date string
inline std::string timestampToString(uint64_t timestamp) {
    std::time_t timeT = timestamp;
    std::tm* timeStruct = std::localtime(&timeT);

    char buffer[128];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeStruct);

    return buffer;
}

// Function to convert date string to uint64 timestamp
inline uint64_t stringToTimestamp(const std::string&time_str) {
    std::tm tm = {};
    std::stringstream ss(time_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::time_t time = std::mktime(&tm);
    if (time == -1) {
        return 0;
    }
    return static_cast<uint64_t>(time);
}
