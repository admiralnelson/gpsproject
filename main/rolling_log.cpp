#include "rolling_log.h"
#include "filesystem.h"
#include "fstream"
#include "iostream"
#include "esp_log.h"

RollingLog& RollingLog::Get()
{
	static RollingLog instance;
	return instance;
}

bool RollingLog::Configure(const std::string &filename, const std::string &header, uint32_t maxLength)
{
	std::string newFileName = "/data/logs/" + filename;
	bool existed = FileSystem::Get().IsFileExists(newFileName.c_str());
	if (!existed) {
		
		RollingLog::LogData log;
		log.Path = newFileName;
		log.Header = header;
		this->ClearLog(log.Header);
		log.Header += "\n";
		log.MaxLength = maxLength;
		log.LineCount = 1;
		this->rollingData.insert({ filename, log });


		FileSystem::Get().WriteFile(newFileName.c_str(), log.Header);
		
		const RollingLog::LogData& data = this->rollingData[filename];
		ESP_LOGI("rolling logger", "inserted new entry %s", data.Path.c_str());
		return true;
	}
	else
	{
		RollingLog::LogData log;
		log.Path = newFileName;
		log.Header = header;
		log.MaxLength = maxLength;
		log.LineCount = 0;
		this->rollingData[filename] = log;
		this->rollingData[filename].LineCount = this->CountLines(filename);

		ESP_LOGI("rolling logger", "already inserted an entry %s", log.Path.c_str());
		return true;
	}

	return false;
}

uint32_t RollingLog::CountLines(const std::string &filename) 
{
	if (!this->IsExist(filename))
	{
		return -1;
	}

	const RollingLog::LogData &data = this->rollingData[filename];

	std::ifstream file(data.Path);
	std::string line;
	uint32_t lineCount = 0;
	while (std::getline(file, line))
	{
		lineCount++;
	}
	return lineCount;
}

uint32_t RollingLog::CountNewLines(const std::string& string)
{
	int newLineCount = 0;
	for (char c : string) {
		if (c == '\n') {
			newLineCount++;
		}
	}
	return newLineCount;
}

void RollingLog::ClearNewLines(std::string& data)
{
	std::size_t last_non_newline = data.find_last_not_of('\n');
	if (last_non_newline != std::string::npos) {
		data.resize(last_non_newline + 1);
	}
	else {
		data.clear();
	}
}

RollingLog::RollingLog()
{
}

bool RollingLog::IsExist(const std::string &filename)
{
	bool foundInMap = this->rollingData.count(filename) > 0;
	
	return foundInMap;
}

bool RollingLog::WriteLog(const std::string &filename, std::string &data)
{
	if (!this->IsExist(filename)) return false;

	const RollingLog::LogData& file = this->rollingData[filename];
	if (file.LineCount >= file.MaxLength) {
		this->ClearLog(filename);
	}

	if (data.back() != '\n') data += "\n";
	FileSystem::Get().AppendFile(file.Path.c_str(), data);
	this->rollingData[filename].LineCount += 1;

	return true;
}

bool RollingLog::ClearLog(const std::string& filename)
{
	if (!this->IsExist(filename)) return false;

	const RollingLog::LogData& file = this->rollingData[filename];
	FileSystem::Get().WriteFile(file.Path.c_str(), file.Header);
	this->rollingData[filename].LineCount = 1;

	return true;
}

bool RollingLog::GetLog(const std::string& filename, std::string& output)
{
	if (!this->IsExist(filename)) return false;

	const RollingLog::LogData& file = this->rollingData[filename];

	FileSystem::Get().LoadFile(filename.c_str(), output);
	return false;
}

RollingLog::LogData RollingLog::GetMetadata(const std::string& filename, bool& IsFound)
{
	if (!this->IsExist(filename))
	{
		IsFound = false;
		ESP_LOGW("rolling logger", "meta data not found %s", filename.c_str());
		return RollingLog::LogData();
	}

	IsFound = true;
	return this->rollingData[filename];
}

