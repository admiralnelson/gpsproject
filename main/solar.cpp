#include "common.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include "display.h"
#include <fstream>
#include <streambuf>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "sdkconfig.h"
#include "iostream"
#include "pins.h"

#ifdef __INTELLISENSE__
#pragma diag_suppress 20
#endif

#include "power_monitor.h"

#define TAG "app"
extern "C"
{
	void app_main(void);
}

#pragma pack(push, 1)
struct TestRGB
{
	int R, G, B, A;
};
#pragma pack(pop)

struct SmallStructTest
{
	int R, G, B;
};


class X {
	int R, G, B;
};

esp_err_t ConfigureStdin(void)
{
	static bool configured = false;
	if (configured) {
		return ESP_OK;
	}
	// Initialize VFS & UART so we can use std::cout/cin
	setvbuf(stdin, NULL, _IONBF, 0);
	/* Install UART driver for interrupt-driven reads and writes */
	ESP_ERROR_CHECK(uart_driver_install((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM,
		256, 0, 0, NULL, 0));
	/* Tell VFS to use UART driver */
	esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);
	esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
	/* Move the caret to the beginning of the next line on '\n' */
	esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);
	configured = true;
	return ESP_OK;
}

const uint32_t PIN_ENABLE_A = 14;
const uint32_t PIN_ENABLE_B = 32;
const uint32_t PIN_CHANNEL_A_FORWARD = 27;
const uint32_t PIN_CHANNEL_A_BACKWARD = 26;
const uint32_t PIN_CHANNEL_B_FORWARD = 25;
const uint32_t PIN_CHANNEL_B_BACKWARD = 33;
const int FREQUENCY = 30000;



void app_main()
{
	ConfigureStdin();

	PWMPin MotorForwardA(PIN_CHANNEL_A_FORWARD, FREQUENCY, 20);
	PWMPin MotorBackwardA(PIN_CHANNEL_A_BACKWARD, FREQUENCY, 20);
	PWMPin MotorForwardB(PIN_CHANNEL_B_FORWARD, FREQUENCY, 20);
	PWMPin MotorBackwardB(PIN_CHANNEL_B_BACKWARD, FREQUENCY, 20);

	std::string command;
	std::cout << "enter your command";
	std::cin >> command;

	PinFunctions::EnablePin(32);

	std::cout << "enter a command milord: \n";
	while (true)
	{
		if (command != "") 
		{
			if (command == "move A forward")
			{
				MotorBackwardA.Pause(true);
				MotorForwardA.SetDutyCycle(100);
			}
			else if (command == "move A backward")
			{
				MotorForwardA.Pause(true);
				MotorBackwardA.SetDutyCycle(100);
			}
			else if (command == "stop A")
			{
				MotorBackwardA.Pause(true);
				MotorForwardA.Pause(true);
			}
			else if (command == "move B forward")
			{
				MotorBackwardB.Pause(true);
				MotorBackwardB.SetDutyCycle(100);
			}
			else if (command == "move B backward")
			{
				MotorForwardB.Pause(true);
				MotorBackwardB.SetDutyCycle(100);
			}

			else if (command == "stop B")
			{
				MotorBackwardB.Pause(true);
				MotorForwardB.Pause(true);
			}
			command = "";
		}
		else
		{
			std::cout << "enter a command milord: \n";
			std::cin >> command;
		}

		//std::this_thread::sleep_for(std::chrono::milliseconds(200));

	}

}