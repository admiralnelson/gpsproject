#pragma once
#include <string>
#include <vector>
#include "cjsoncpp.h"
#define MAX_FILES 10

struct FileInfo : public JsonSerialisable
{
	std::string FileName;
	std::string Path;
	long int Size;
	bool bIsDir;
	time_t ModifiedTime;
	std::string GetFileNameWithPath() const;
	cjsonpp::JSONObject ToJsonObject() const override;
	bool FromJsonObject(const cjsonpp::JSONObject& Object) noexcept override;
};

class FileSystem
{
public:
	static FileSystem& Get();

	bool FormatInternalPartition();
	bool EraseFile(const char* Path);

	/*
		Currently doesn't work
	*/
	bool MakeDir(const char* Path);
	/*
		Currently doesn't work
	*/
	bool EraseDirectory(const char* Path);

	/*
		Will erase every child recursively under a directory
	*/
	bool RenameFile(const char* OldFile, const char* NewFile);
	bool IsDirectoryExists(const char* Path) const;
	bool IsFileExists(const char* Path) const;
	void PrintDirs(const char* Path) const;
	bool GetListOfFiles(const char* Path, std::vector<FileInfo>& Files) const;
	int GetFreeDiscSpace() const;
	int GetDiscSize() const;
	bool IsMounted() const;
	bool LoadFile(const char* Path, std::string& Result);
	bool LoadFileInChunk(const char* Path, std::string& Result, int Position = 0, int Length = -1);
	bool WriteFile(const char* Path, const std::string& Data);
	bool AppendFile(const char* Path, const std::string& Data);

private:
	bool InitLittleFS();
	bool EraseDirectoryRecursive(std::string Path, int &AccumulatedDepth);

private:
	bool bSPIFFSHasInited = false;

private:
	FileSystem();
	FileSystem(FileSystem const&) = delete;
	void operator=(FileSystem const&) = delete;
};