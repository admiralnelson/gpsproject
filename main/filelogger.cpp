#include "filelogger.h"
#include <algorithm>
#include <limits>
#include "filesystem.h"


#define MIN_APP_PARTITION_SIZE  1500000

time_t ParseFileNameToTime(const std::string& Filename)
{
	TimeStructure tm;
	sscanf(Filename.c_str(), "%d.%d.%d %d.%d_.txt", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min);
	tm.tm_year = tm.tm_year - 1900;
	time_t ToTimeT = mktime(&tm);
	return ToTimeT;
}

FileLogger& FileLogger::Get()
{
	static FileLogger instance;
	return instance;
}

void FileLogger::WriteLog(const std::string& Log, const std::string& LogMessage)
{
	if (FileSystem::Get().GetFreeDiscSpace() < MIN_APP_PARTITION_SIZE)
	{
		EraseOldestFile();
	}
	std::string Message = TimeToString(time(0)) + " " + Log + "  " + LogMessage + "\n";
	FileSystem::Get().AppendFile(FileName.data(), Message);
}

void FileLogger::WriteLogWarn(const std::string& Log, const std::string& LogMessage)
{
	WriteLog("Warning: " + Log, LogMessage);
}

void FileLogger::WriteLogError(const std::string& Log, const std::string& LogMessage)
{
	WriteLog("Error: " + Log, LogMessage);
}

void FileLogger::WriteLogInfo(const std::string& Log, const std::string& LogMessage)
{
	WriteLog("Info: " + Log, LogMessage);
}

void FileLogger::MakeNewLog()
{
	CurrentTime = time(0);
	TimeStructure tm = GetTmNow();
	std::string Name;
	Name += std::to_string(1900 + tm.tm_year) + "." + std::to_string(tm.tm_mon) + "." + std::to_string(tm.tm_mday) + " " + std::to_string(tm.tm_hour) + "." + std::to_string(tm.tm_min) + "_.txt";
	FileName = "/data/logs/" + Name;
	if (!FileSystem::Get().IsDirectoryExists("/data/logs"))
	{
		FileSystem::Get().MakeDir("/data/logs");
		WriteLog("NewLog...", "...Begins");
	}

	int FreeSpace =  0;
	int TotalSpace = 0;
	int SpaceQuota = 0;
	do
	{
		FreeSpace = FileSystem::Get().GetFreeDiscSpace();
		TotalSpace = FileSystem::Get().GetDiscSize();
		SpaceQuota = (int)(TotalSpace * 0.1);
		if (FreeSpace - SpaceQuota < MIN_APP_PARTITION_SIZE)
		{
			EraseOldestFile();
			WriteLog("Log", "Erasing oldest file to freeup space");
		}
	} 
	while (FreeSpace - SpaceQuota < MIN_APP_PARTITION_SIZE);
}

void FileLogger::EraseAWeekOldLogs()
{
	int i = 0;
	std::vector<FileInfo> Files;
	if (FileSystem::Get().GetListOfFiles("/data/logs/", Files))
	{
		for (const auto& f : Files)
		{
			time_t aWeek = time(0) - 604800;
			time_t ToTimeT = ParseFileNameToTime(Files[i].FileName);
			if (ToTimeT < aWeek)
			{
				FileSystem::Get().EraseFile(f.GetFileNameWithPath().c_str());
				i++;
			}
		}
	}
	WriteLogInfo("Log", "Weekly log cleanup. Erased" + std::to_string(i) + " logs");
}

void FileLogger::EraseOldestFile()
{
	int index = 0;
	time_t oldest = std::numeric_limits<time_t>::max();
	std::vector<FileInfo> Files;
	if (FileSystem::Get().GetListOfFiles("/data/logs/", Files))
	{
		int len = Files.size();
		for (size_t i = 0; i < len; i++)
		{
			time_t ToTimeT = ParseFileNameToTime(Files[i].FileName);
			if (ToTimeT < oldest)
			{
				oldest = ToTimeT;
				index = i;
			}
		}
		FileSystem::Get().EraseFile(Files[index].GetFileNameWithPath().c_str());
	}
}

void FileLogger::EraseAll(bool bWriteLogAfterErase)
{
	std::vector<FileInfo> Files;
	if (FileSystem::Get().GetListOfFiles("/data/logs/", Files))
	{
		for (const auto& f : Files)
		{
			FileSystem::Get().EraseFile(f.GetFileNameWithPath().c_str());
		}
	}
	if (bWriteLogAfterErase)
	{
		WriteLogInfo("Log", "Erased all logs. Total " + std::to_string(Files.size()));
	}
}

FileLogger::FileLogger()
{
	MakeNewLog();
	WriteLog("Log", "Logging started");
}