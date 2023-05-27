#include "common.h"
#include "esp_timer.h"
#include "thread"
#include "regex"
#include "sys/time.h"
#include "ctime"

bool ChangeSystemDateTime(long unixEpochSeconds)
{
    timeval tv;
    tv.tv_sec = unixEpochSeconds;
    tv.tv_usec = 0;
    if (settimeofday(&tv, NULL) != 0) {
        return false;
    }
    return true;
}

uint64_t GetSystemMilliseconds()
{
	return esp_timer_get_time() / 1000;
}

void Delay(uint64_t howLongInMilliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(howLongInMilliseconds));
}

bool SetDateAndTime(const std::string& dateTimeString)
{
    struct tm tm;
    if (strptime(dateTimeString.c_str(), "%Y-%m-%dT%H:%M:%S", &tm) == NULL)
    {
        return false;
    }
    time_t unixEpochSeconds = mktime(&tm);
    return ChangeSystemDateTime(unixEpochSeconds);

}

long GetCurrentSystemDateTime()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0)
    {
        return -1;
    }
    return tv.tv_sec;
}

std::string GetCurrentSystemDateTimeAsString()
{
    time_t unixEpochSeconds = GetCurrentSystemDateTime();
    tm* tm = localtime(&unixEpochSeconds);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", tm);
    return std::string(buffer);
}

DateTime GetCurrentSystemDateTimeStruct()
{
    time_t unixEpochSeconds = GetCurrentSystemDateTime();
    tm* tm = localtime(&unixEpochSeconds);
    DateTime dt;
    dt.year = tm->tm_year + 1900;
    dt.month = tm->tm_mon + 1;
    dt.day = tm->tm_mday;
    dt.hour = tm->tm_hour;
    dt.minute = tm->tm_min;
    dt.second = tm->tm_sec;
    return dt;
}

