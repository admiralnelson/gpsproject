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
#include "motor.h"

#ifdef __INTELLISENSE__
#pragma diag_suppress 20
#endif

#include "power_monitor.h"

#define TAG "app"
extern "C"
{
	void app_main(void);
}

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

//move.A.forward 01.11.34   : 20 derajat
// move.A.backward 01.12.79 : 20 derajat
// 
//move.B.forward 01.13.00   : 20 derajat
//move.B.backward 01.10.51  : 25 derajat
void app_main()
{
	ConfigureStdin();

	std::string command;
	int DegA = 0;
	int DegB = 0;
	//MotorController::Get().Reset();

	std::cout << "enter a command milord: ";
	while (true)
	{
		if (command != "") 
		{
			if (command == "deg(a,-20)")
			{
				MotorController::Get().SetAToDeg(-20);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(a,-15)")
			{
				MotorController::Get().SetAToDeg(-15);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(a,-10)")
			{
				MotorController::Get().SetAToDeg(-10);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(a,0)")
			{
				MotorController::Get().SetAToDeg(0);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(a,2)")
			{
				MotorController::Get().SetAToDeg(2);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(a,-2)")
			{
				MotorController::Get().SetAToDeg(-2);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(a,5)")
			{
				MotorController::Get().SetAToDeg(5);
				std::cout << "done" << std::endl;
			}

			else if (command == "deg(a,10)")
			{
				MotorController::Get().SetAToDeg(10);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(a,15)")
			{
				MotorController::Get().SetAToDeg(15);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(a,20)")
			{
				MotorController::Get().SetAToDeg(20);
				std::cout << "done" << std::endl;
			}
			else if (command == "a+")
			{
				MotorController::Get().StepAForward();
				std::cout << "done" << std::endl;
			}
			else if (command == "a-")
			{
				MotorController::Get().StepABackward();
				std::cout << "done" << std::endl;
			}
			else if (command == "b+")
			{
				MotorController::Get().StepBForward();
				std::cout << "done" << std::endl;
			}
			else if (command == "b-")
			{
				MotorController::Get().StepBBackward();
				std::cout << "done" << std::endl;
			}
			else if (command == "clear")
			{
				MotorController::Get().DebugClearMemory();
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(a,10)")
			{
				MotorController::Get().SetAToDeg(10);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(a,0)")
			{
				MotorController::Get().SetAToDeg(0);
				std::cout << "done" << std::endl;
			}
			//else if (command == "deg(b, 20)")
			//{
			//	MotorController::Get().SetBToDeg(20);
			//	std::cout << "done" << std::endl;
			//}
			//else if (command == "deg(b,-25)")
			//{
			//	MotorController::Get().SetBToDeg(-25);
			//	std::cout << "done" << std::endl;
			//}
			//else if (command == "reset")
			//{
			//	MotorController::Get().Reset();
			//	std::cout << "done" << std::endl;
			//}
			else
			{

			}
			command = "";
		}
		else
		{
			std::cout << "enter a command milord: " << std::endl;
			std::cin >> command;
		}

		//std::this_thread::sleep_for(std::chrono::milliseconds(200));

	}

}