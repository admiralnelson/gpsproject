#include "filesystem.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include <ctime>
#include <algorithm>
#include <errno.h>
#include <fstream>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#define TAG "filesystem"
#define PARTITION_LABEL "data"
#define MAX_DEPTH 5

FileSystem& FileSystem::Get()
{
	static FileSystem instance;
	return instance;
}

bool FileSystem::FormatInternalPartition()
{
    esp_littlefs_format(PARTITION_LABEL);
    return false;
}

bool FileSystem::EraseFile(const char* Path)
{
    bool ret =  remove(Path) == 0;
    if (!ret)
    {
        ESP_LOGE(TAG, "remove %s, errno returned %s", Path, strerror(errno));
    }
    return ret;
}

bool FileSystem::MakeDir(const char* Path)
{
    bool ret = mkdir(Path, 0777) == 0;
    if (!ret)
    {
        ESP_LOGE(TAG, "mkdir %s, errno returned %s", Path, strerror(errno));
    }
    return ret;
}


int unlink_cb(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}


bool FileSystem::EraseDirectory(const char* Path)
{
    std::string StartingPath = Path;
    if (StartingPath.back() == '/')
    {
        StartingPath.pop_back();
    }
    int AccumulatedDepth = 0;
    bool Result = EraseDirectoryRecursive(StartingPath, AccumulatedDepth);
    return Result;
}


bool FileSystem::EraseDirectoryRecursive(std::string Path, int& AccumulatedDepth)
{
    std::vector<FileInfo> Files;
    bool Result;
    if (GetListOfFiles(Path.data(), Files))
    {
        for (const auto& f : Files)
        {
            if (f.bIsDir)
            {
                Path += "/" + f.FileName;
                AccumulatedDepth++;
                Result = EraseDirectoryRecursive(Path, AccumulatedDepth);
            }
            else
            {
                EraseFile(f.GetFileNameWithPath().data());
            }
        }
    }

    return EraseFile(Path.data());
}


bool FileSystem::RenameFile(const char* Path, const char* NewPath)
{
    return rename(Path, NewPath) == 0;
}

bool FileSystem::IsDirectoryExists(const char* Path) const
{
    DIR* dir = opendir(Path);
    if (!dir)
    {
        return false;
    }
    closedir(dir);
    return true;
}

bool FileSystem::IsFileExists(const char* Path) const
{
    return (access(Path, F_OK) == 0);
}

void FileSystem::PrintDirs(const char* Path) const
{
    std::vector<FileInfo> Files;
    Files.reserve(10);
    if (GetListOfFiles(Path, Files))
    {
        ESP_LOGI(TAG, "Listing on: %s", Path);
        for (const auto& f : Files)
        {
            std::string HumanDate(std::ctime(&f.ModifiedTime));
            HumanDate.erase(std::remove(HumanDate.begin(), HumanDate.end(), '\n'), HumanDate.end());
            if (f.bIsDir)
            {
                ESP_LOGI(TAG, "Dir: %s  | Date Modified: %s", f.FileName.data(), HumanDate.data());
            }
            else
            {
                ESP_LOGI(TAG, "File: %s  | Date Modified: %s, Size: %ld Byte", f.FileName.data(), HumanDate.data(), f.Size);
            }
        }
    }
}

bool FileSystem::GetListOfFiles(const char* Path, std::vector<FileInfo>& Files) const
{
    Files.clear();

    struct dirent* entry;
    DIR* dir = opendir(Path);
    if (!dir)
    {
        ESP_LOGE(TAG, "Failed to stat dir : %s (perhaps wrong path)", Path);
        return false;
    }

    entry = readdir(dir);
    while (entry != nullptr)
    {
        FileInfo File;
        std::string FilePath = Path;
        File.Path = Path;
        if (FilePath.find('/') != std::string::npos)
        {
            FilePath += '/';
            File.Path += '/';
        }

        FilePath += entry->d_name;
        struct stat FileStat;
        memset(&FileStat, 0, sizeof(FileStat));
        stat(FilePath.data(), &FileStat);
        File.FileName = entry->d_name;
        if (entry->d_type == DT_DIR)
        {
            File.bIsDir = true;
        }
        else
        {
            File.bIsDir = false;
            File.ModifiedTime = FileStat.st_mtime;
            File.Size = FileStat.st_size;
        }

        Files.push_back(File);
        entry = readdir(dir);
    }
    closedir(dir);
    return true;
}

int FileSystem::GetFreeDiscSpace() const
{
    size_t total = 0, used = 0;
    esp_err_t ret = esp_littlefs_info(PARTITION_LABEL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get  partition information (%s)", esp_err_to_name(ret));
        return -1;
    }
    return total - used;
}

int FileSystem::GetDiscSize() const
{
    size_t total = 0, used = 0;
    esp_err_t ret = esp_littlefs_info(PARTITION_LABEL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get  partition information (%s)", esp_err_to_name(ret));
        return -1;
    }
    return total;
}

bool FileSystem::IsMounted() const
{
    return esp_littlefs_mounted(PARTITION_LABEL);
}

bool FileSystem::LoadFile(const char* Path, std::string& Result)
{
    return LoadFileInChunk(Path, Result, 0, -1);
}

bool FileSystem::LoadFileInChunk(const char* Path, std::string& Result, int Position, int Length)
{
    if (Position < 0)
    {
        return false;
    }

    FILE* file = nullptr;
    file = fopen(Path, "r");
    if(file != nullptr)
    {
        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        if (Length > size || Length < 0)
        {
            Length = size;
        }

        fseek(file, Position, SEEK_SET);
        ESP_LOGI(TAG, "size %d", Length);
        Result.resize(Length);
        ESP_LOGI(TAG, "now read");
        int totalRead = fread(&Result[0], 1, Length, file);

        fclose(file);

        ESP_LOGI(TAG, "sucessfully read file as chunk %s begin %d, End %d, Length %d", Path, Position, Position + Length, totalRead);
        return true;
    }
    ESP_LOGE(TAG, "fail to read file as chunk %s begin %d, End %d, reason %s", Path, Position, Position + Length, strerror(errno));
    return false;
}

bool FileSystem::WriteFile(const char* Path, const std::string& Data)
{
    FILE* file = nullptr;
    file = fopen(Path, "w");
    if (file != nullptr)
    {
        int totalWrite =fwrite(Data.data(), sizeof(char), Data.size(), file);
        fclose(file);

        ESP_LOGI(TAG, "sucessfully write file %s, Length %d", Path, totalWrite);
        return true;
    }
    ESP_LOGE(TAG, "fail to write file %s, reason %s", Path, strerror(errno));
    return false;
}

bool FileSystem::AppendFile(const char* Path, const std::string& Data)
{
    FILE* file = nullptr;
    file = fopen(Path, "a");
    if (file != nullptr)
    {
        int totalWrite = fwrite(Data.data(), sizeof(char), Data.size(), file);
        fclose(file);

        ESP_LOGI(TAG, "sucessfully append file %s, Length %d", Path, totalWrite);
        return true;
    }
    ESP_LOGE(TAG, "fail to append file %s, reason %s", Path, strerror(errno));
    return false;
}

bool FileSystem::GetFileInfo(const char* Path, FileInfo& fileinfo)
{
    //if (!IsFileExists(Path)) return false;

    FileInfo File;
    std::string FilePath = Path;
    File.Path = Path;
    struct stat FileStat;
    memset(&FileStat, 0, sizeof(FileStat)); 
    stat(FilePath.data(), &FileStat);
    File.FileName = Path;

    File.bIsDir = false;
    File.ModifiedTime = FileStat.st_mtime;
    File.Size = FileStat.st_size;
    

    fileinfo = File;
    return true;
}

bool FileSystem::InitLittleFS()
{
    if (bSPIFFSHasInited)
    {
        return true;
    }

    esp_vfs_littlefs_conf_t  conf = {
	  .base_path = "/data",
	  .partition_label = PARTITION_LABEL,
	  .format_if_mount_failed = false,
      .dont_mount = false,
	};
	esp_err_t ret = esp_vfs_littlefs_register(&conf);
	
    if (ret != ESP_OK) 
    {
        if (ret == ESP_FAIL) 
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND) 
        {
            ESP_LOGE(TAG, "Failed to find partition");
        }
        else 
        {
            ESP_LOGE(TAG, "Failed to initialize (%s)", esp_err_to_name(ret));
        }
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(PARTITION_LABEL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get partition information (%s)", esp_err_to_name(ret));
        return false;
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
        return true;
    }
}

FileSystem::FileSystem()
{
    bSPIFFSHasInited = InitLittleFS();
    ESP_LOGI(TAG, "initialised");
}

std::string FileInfo::GetFileNameWithPath() const
{
    return Path + FileName;
}

cjsonpp::JSONObject FileInfo::ToJsonObject() const
{
    cjsonpp::JSONObject Object;
    Object.set<std::string>("FileName", FileName);
    Object.set<std::string>("Path", Path);
    Object.set<int64_t>("Size", Size);
    Object.set<bool>("bIsDir", bIsDir);
    Object.set<int64_t>("ModifiedTime", ModifiedTime);

    return Object;
}

bool FileInfo::FromJsonObject(const cjsonpp::JSONObject& Object) noexcept
{
    try
    {
        FileName = Object.get<std::string>("FileName");
        Path = Object.get<std::string>("Path");
        Size = (long int)Object.get<int64_t> ("Size");
        bIsDir = Object.get<bool>("bIsDir");
        ModifiedTime = (long int)Object.get<int64_t>("ModifiedTime");
        return true;
    }
    catch (const cjsonpp::JSONError& e)
    {
        ESP_LOGE(TAG, "%s", e.what());
        return false;
    }
}