#pragma once
#include "common.h"
#include "map"

class RollingLog {
private:
	struct LogData {
		std::string Header;
		std::string Path;
		uint32_t MaxLength = 0;
		uint32_t LineCount = 0;
	};

public:
	static RollingLog& Get();
	bool Configure(const std::string &filename, const std::string &header, uint32_t maxLength);
	bool IsExist(const std::string &filename);
	bool WriteLog(const std::string &filename, std::string &data);
	bool ClearLog(const std::string &filename);
	bool GetLog(const std::string &filename, std::string& output);
	LogData GetMetadata(const std::string& filename, bool &IsFound);
	uint32_t CountLines(const std::string &filename);

private:
	uint32_t CountNewLines(const std::string &string);
	void ClearNewLines(std::string& data);

private:
	std::map<std::string, LogData> rollingData;

	RollingLog(RollingLog const&) = delete;
	void operator=(RollingLog const&) = delete;
	RollingLog();
};