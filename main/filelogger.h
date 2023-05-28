#pragma once
#include "common.h"

class FileLogger
{
public:
	static FileLogger& Get();
	void WriteLog(const std::string& Log, const std::string& LogMessage);
	void WriteLogWarn(const std::string& Log, const std::string& LogMessage);
	void WriteLogError(const std::string& Log, const std::string& LogMessage);
	void WriteLogInfo(const std::string& Log, const std::string& LogMessage);
	void MakeNewLog();
	void EraseAWeekOldLogs();
	void EraseOldestFile();
	void EraseAll(bool bWriteLogAfterErase = true);
private:
	std::string FileName;
	time_t CurrentTime;
	FileLogger();
	FileLogger(FileLogger const&) = delete;
	void operator=(FileLogger const&) = delete;
};