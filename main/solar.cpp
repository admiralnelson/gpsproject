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
#include "gps.h"
#include <regex> 
#include "solartrackingthread.h"
#include "filesystem.h"
#include "pages_controller.h"
#include "api_controller.h"
#include "logger.h"

#include "wifi.h"
#include "webserver.h"

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

	//I2CMaster I2CDeviceManager(21, 22, I2C_NUM_0);
	//I2CDeviceManager::Get();

	Wifi::Get().StartAP("SolarTrack system");
	//Logger::Start();
	HTTPServer Server(80, 40);
	PagesController::Get().Register(Server);
	ApiController::Get().Register(Server);
	

	Server.RegisterURI("/helloworld", [](HTTPRequest& Request)
	{
		Request.SendReply("hello world");
		std::cout << "hello world was opened" << std::endl;
	});

	Wifi::Get().RegisterWiFiEvent(WIFI_EVENT_STA_CONNECTED, "wifi connected",
	[](void* Data)
	{
		ESP_LOGI(TAG, "wifi connected");
	});


	std::cout << "enter a command milord: ";
	while (true)
	{
		if (command != "") 
		{
			std::regex pattern(R"(^setdate:(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}))");
			std::smatch match;

			//test untuk motor A
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

			//test untuk motor B
			else if (command == "deg(b,-20)")
			{
				MotorController::Get().SetBToDeg(-20);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(b,-15)")
			{
				MotorController::Get().SetBToDeg(-15);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(b,-10)")
			{
				MotorController::Get().SetBToDeg(-10);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(b,0)")
			{
				MotorController::Get().SetBToDeg(0);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(b,2)")
			{
				MotorController::Get().SetBToDeg(2);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(b,-2)")
			{
				MotorController::Get().SetBToDeg(-2);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(b,5)")
			{
				MotorController::Get().SetBToDeg(5);
				std::cout << "done" << std::endl;
			}

			else if (command == "deg(b,10)")
			{
				MotorController::Get().SetBToDeg(10);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(b,15)")
			{
				MotorController::Get().SetBToDeg(15);
				std::cout << "done" << std::endl;
			}
			else if (command == "deg(b,20)")
			{
				MotorController::Get().SetBToDeg(20);
				std::cout << "done" << std::endl;
			}

			//test unutk motor A/B per degrees
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
			else if (command == "resetmotor")
			{
				MotorController::Get().SetAToDeg(0);
				MotorController::Get().SetBToDeg(0);
				std::cout << "done" << std::endl;
				}
			else if (command == "clear")
			{
				MotorController::Get().DebugClearMemory();
				std::cout << "done" << std::endl;
			}
			else if (command == "gpstest")
			{
				std::cout << "gps enabled" << std::endl;
				Gps::Get();
			}
			else if (command == "hidegps")
			{
				std::cout << "hide gps debug" << std::endl;
				Gps::Get().ShowDebug(false);
			}
			else if (command == "showgps")
			{
				std::cout << "show gps debug" << std::endl;
				Gps::Get().ShowDebug(true);
			}
			else if (command == "showdate")
			{
				std::cout << "showdate: " << GetCurrentSystemDateTimeAsString() << std::endl;
			}
			else if (command == "startsolaropen")
			{
				SolarTrackingClosedLoop::Get().Stop();
				SolarTrackingOpenLoop::Get().Start();
			}
			else if (command == "stopsolaropen")
			{
				SolarTrackingOpenLoop::Get().Stop();
			}
			else if (command == "ldrcheck")
			{
				int ldr1 = PinFunctions::ReadAnalog2(ADC2_CHANNEL_0);
				int ldr2 = PinFunctions::ReadAnalog2(ADC2_CHANNEL_3);
				int ldr3 = PinFunctions::ReadAnalog1(ADC1_CHANNEL_3);
				int ldr4 = PinFunctions::ReadAnalog1(ADC1_CHANNEL_0);

				std::cout << "in mV: ldr1 " << ldr1 << ", ldr2 " << ldr2 << ", ldr3 " << ldr3 << ", ldr4 " << ldr4 << std::endl;

			}
			else if (command == "testvoltage")
			{
				int voltageADC = PinFunctions::ReadAnalog1(ADC1_CHANNEL_6);
				std::cout << "in mV " << voltageADC<< std::endl;

			}
			else if (command == "h")
			{
				std::thread t([] {
					while (true) {
						float current = PowerMonitor::Get().Current();
						float voltage = PowerMonitor::Get().Voltage();
						float power = PowerMonitor::Get().Power();

						std::cout << "A : " << current << "  V:  " << voltage << " P : " << power << std::endl;
						Delay(100);
					}
					});
				t.join();
			}
			else if (command == "getcurrent")
			{
				float voltage = PowerMonitor::Get().Current();
				std::cout << "V : " << voltage << std::endl;
			}
			else if (command == "startsolarclosed")
			{
				SolarTrackingOpenLoop::Get().Stop();
				SolarTrackingClosedLoop::Get().Start(); 
			}
			else if (command == "stopsolarclosed")
			{
				SolarTrackingClosedLoop::Get().Stop();
			}
			else if (command == "dir")
			{
				FileSystem::Get().PrintDirs("/");
				FileSystem::Get().PrintDirs("/data/");
				std::cout << "free space " << FileSystem::Get().GetFreeDiscSpace() << "/" << FileSystem::Get().GetDiscSize() << std::endl;
			}
			else if (command == "openfile")
			{
				std::string data;
				bool bExisst = FileSystem::Get().LoadFile("/data/index.htm", data);;
				if (bExisst)
				{
					std::cout << data << std::endl;
				}
				else
				{
					std::cout << "data not found" << std::endl;
				}
			}
			else if (std::regex_match(command, match, pattern))
			{
				std::string dateTimeString = match[1];
				bool bSuccess = SetDateAndTime(dateTimeString);
				if (bSuccess)
				{
					std::cout << "set date and time is done milord. current system time is " << GetCurrentSystemDateTimeAsString() << std::endl;
				}
			}
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