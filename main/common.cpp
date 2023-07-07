#include "common.h"
#include "esp_timer.h"
#include "thread"
#include "regex"
#include "sys/time.h"
#include "ctime"

#include "common.h"
#include "esp_log.h"
#include "esp_system.h"
#include "soc/rtc.h"
#include "hal/wdt_hal.h"
#include <cstring>
#include <algorithm>
#include <esp_random.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


#define LOG_TAG "	"

void DisableBrownOutDetector(bool bDisableIt)
{
	if (bDisableIt)
	{
		WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
	}
	else
	{
		WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1);
	}
}

std::string GenerateRandomString(size_t Length)
{
	std::string Result;
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	srand((unsigned)time(NULL) * esp_random());

	Result.reserve(Length);

	for (int i = 0; i < Length; ++i)
	{
		Result += alphanum[rand() % (sizeof(alphanum) - 1)];
	}


	return Result;
}

bool StringContainsChar(const std::string& A, char C)
{
	return A.find(C) != std::string::npos;
}

int Clamp(int A, int Min, int Max)
{
	if (A <= Min) return Min;
	if (A >= Max) return Max;
	return A;
}

bool IsBetween(int A, int Begin, int End)
{
	return (Begin >= A && A <= End);
}

std::string SubStringBeforeChar(std::string const& String, char A)
{
	std::string::size_type pos = String.find(A);
	if (pos != std::string::npos)
	{
		return String.substr(0, pos);
	}
	else
	{
		return "";
	}
}

std::string TimeToString(time_t Time)
{
	std::string out = ctime(&Time);
	std::replace(out.begin(), out.end(), '\r', ' ');
	std::replace(out.begin(), out.end(), '\n', ' ');
	return out;
}

void PrintTime(const tm& Time)
{
	char buff[20];
	strftime(buff, sizeof(buff), "%Y %b %d %H:%M", &Time);
	ESP_LOGI(LOG_TAG, "time: %s", buff);
}

void PrintTime(const time_t Time)
{
	tm TimeS;
	memcpy(&TimeS, localtime(&Time), sizeof(TimeS));
	char buff[20];
	strftime(buff, sizeof(buff), "%Y %b %d %H:%M", &TimeS);
	ESP_LOGI(LOG_TAG, "time: %s", buff);
}

tm GetTmNow()
{
	time_t Now = time(0);
	tm TimeStruct1;
	std::memcpy(&TimeStruct1, localtime(&Now), sizeof(tm));
	return TimeStruct1;
}

void ColdReboot()
{
	int delay_ms = 0;
	uint32_t slow_clk_freq = rtc_clk_slow_freq_get_hz();
	wdt_hal_context_t rtc_wdt_ctx = { .inst = WDT_RWDT, .rwdt_dev = &RTCCNTL };
	wdt_hal_write_protect_disable(&rtc_wdt_ctx);
	wdt_hal_set_flashboot_en(&rtc_wdt_ctx, false);
	wdt_hal_write_protect_enable(&rtc_wdt_ctx);

	wdt_hal_init(&rtc_wdt_ctx, WDT_RWDT, 0, false);

	wdt_hal_write_protect_disable(&rtc_wdt_ctx);
	wdt_hal_config_stage(&rtc_wdt_ctx, WDT_STAGE0, ((delay_ms) * (slow_clk_freq / 1000)), WDT_STAGE_ACTION_RESET_RTC);
	wdt_hal_enable(&rtc_wdt_ctx);
	wdt_hal_write_protect_enable(&rtc_wdt_ctx);
	for (;;);
}

uint32_t TotalFreeHeap()
{
	return esp_get_free_heap_size();
}

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

cjsonpp::JSONObject DateTime::ToJsonObject() const
{
	cjsonpp::JSONObject object;
	object.set("year", this->year);
	object.set("month", this->month);
	object.set("day", this->day);
	object.set("hour", this->hour);
	object.set("minute", this->minute);
	object.set("second", this->second);

	return object;
}

bool DateTime::FromJsonObject(const cjsonpp::JSONObject& Object) noexcept
{
	try
	{
		if (Object.has("year"))
		{
			this->year = Object.get<int>("year");
		}
		if (Object.has("month"))
		{
			this->year = Object.get<int>("month");
		}
		if (Object.has("day"))
		{
			this->year = Object.get<int>("day");
		}
		if (Object.has("hour"))
		{
			this->year = Object.get<int>("hour");
		}
		if (Object.has("minute"))
		{
			this->year = Object.get<int>("minute");
		}
		if (Object.has("second"))
		{
			this->year = Object.get<int>("second");
		}
		return true;
	}
	catch (const std::exception& e)
	{
		ESP_LOGE(LOG_TAG, "error parsing datetime struct %s", e.what());
		return false;
	}
}
