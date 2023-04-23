#include "common.h"
#include "esp_timer.h"
#include "thread"

uint64_t GetSystemMilliseconds()
{
	return esp_timer_get_time() / 1000;
}

void Delay(uint64_t howLongInMilliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(howLongInMilliseconds));
}
