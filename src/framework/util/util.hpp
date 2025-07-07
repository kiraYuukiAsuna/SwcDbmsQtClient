#pragma once

#include <google/protobuf/timestamp.pb.h>

#include <QStandardPaths>
#include <sstream>
#include <string>
#include <vector>

inline std::string replaceAll(std::string str, const std::string& from,
							  const std::string& to) {
	size_t startPos = 0;
	while ((startPos = str.find(from, startPos)) != std::string::npos) {
		str.replace(startPos, from.length(), to);
		startPos += to.length();  // Move past the last replaced substring
	}
	return str;
}

inline std::vector<std::string> stringSplit(const std::string& str,
											char delim) {
	std::stringstream ss(str);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

inline std::string subreplace(std::string resource_str, std::string sub_str,
							  std::string new_str) {
	std::string::size_type pos = 0;
	while ((pos = resource_str.find(sub_str)) !=
		   std::string::npos)  // 替换所有指定子串
	{
		resource_str.replace(pos, sub_str.length(), new_str);
	}
	return resource_str;
}

inline bool isEarlier(const google::protobuf::Timestamp& t1,
					  const google::protobuf::Timestamp& t2) {
	if (t1.seconds() < t2.seconds()) {
		return true;
	} else if (t1.seconds() > t2.seconds()) {
		return false;
	} else {
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
inline uint64_t stringToTimestamp(const std::string& time_str) {
	std::tm tm = {};
	std::stringstream ss(time_str);
	ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
	std::time_t time = std::mktime(&tm);
	if (time == -1) {
		return 0;
	}
	return static_cast<uint64_t>(time);
}

inline std::string getTempLocation() {
	return QStandardPaths::writableLocation(QStandardPaths::TempLocation)
		.toStdString();
}
