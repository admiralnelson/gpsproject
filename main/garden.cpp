#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <string>
#include <thread>
#include <chrono>

#define TAG "app"


extern "C"
{
	void app_main(void);
}

void app_main()
{
	while (true)
	{
		std::string test("test");
		printf(test.c_str());
		fflush(stdout);
		std::chrono::seconds sec(1);
		std::this_thread::sleep_for(sec);
	}
}