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

const uint32_t PIN_ENABLE_A = 19;
const uint32_t PIN_ENABLE_B = 18;
const uint32_t PIN_CHANNEL_A_FORWARD = 5;
const uint32_t PIN_CHANNEL_A_BACKWARD = 17;
const uint32_t PIN_CHANNEL_B_FORWARD = 16;
const uint32_t PIN_CHANNEL_B_BACKWARD = 4;
const int FREQUENCY = 1000;
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} color_t;
#define LED_RED_PIN         5
#define LED_GREEN_PIN       17
#define LED_BLUE_PIN        16
ledc_timer_config_t ledc_timer = {
	.speed_mode = LEDC_LOW_SPEED_MODE,
	.duty_resolution = LEDC_TIMER_13_BIT,
	.timer_num = LEDC_TIMER_0,
	.freq_hz = 1000,
	.clk_cfg = LEDC_AUTO_CLK
};
ledc_channel_config_t ledc_channel[3];
static void init(void)
{
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
	ledc_channel[0].channel = LEDC_CHANNEL_0;
	ledc_channel[0].gpio_num = LED_RED_PIN;
	ledc_channel[1].channel = LEDC_CHANNEL_1;
	ledc_channel[1].gpio_num = LED_GREEN_PIN;

	ledc_channel[2].channel = LEDC_CHANNEL_2;
	ledc_channel[2].gpio_num = LED_BLUE_PIN;
	for (int i = 0; i < 3; i++)
	{
		ledc_channel[i].speed_mode = LEDC_LOW_SPEED_MODE;
		ledc_channel[i].timer_sel = LEDC_TIMER_0;
		ledc_channel[i].intr_type = LEDC_INTR_DISABLE;
		ledc_channel[i].duty = 0;
		ledc_channel[i].hpoint = 0;

		ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel[i]));
	}
}
static void setColor(int dutycycle)
{
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, dutycycle));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, dutycycle));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, dutycycle));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2));
}

void app_main()
{
	ConfigureStdin();
	//init();
	//int i = 0;
	//while (1) {
	//	setColor(i);
	//	vTaskDelay(pdMS_TO_TICKS(200));
	//	i++;
	//	if (i > 1023) i = 0;
	//}

	PWMPin MotorForwardA(PIN_CHANNEL_A_FORWARD, FREQUENCY);
	PWMPin MotorBackwardA(PIN_CHANNEL_A_BACKWARD, FREQUENCY);
	PWMPin MotorForwardB(PIN_CHANNEL_B_FORWARD, FREQUENCY);
	PWMPin MotorBackwardB(PIN_CHANNEL_B_BACKWARD, FREQUENCY);

	int i = 0;
	while (true)
	{
		MotorForwardA.SetDutyCycle(i);
		MotorForwardB.SetDutyCycle(i);

		MotorBackwardA.SetDutyCycle(i);
		MotorBackwardB.SetDutyCycle(i);

		i++;
		if (i >= 100) i = 0;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	//MotorBackwardA.Pause(true);
	//MotorBackwardB.Pause(true);
	//MotorForwardA.Pause(true);
	//MotorForwardB.Pause(true);

	//PinFunctions::EnablePin(PIN_ENABLE_A);
	//PinFunctions::EnablePin(PIN_ENABLE_B);

	//std::string command;

	//std::cout << "enter a command milord: ";
	//while (true)
	//{
	//	if (command != "") 
	//	{
	//		if (command == "move.A.forward")
	//		{
	//			MotorBackwardA.Pause(true);
	//			MotorForwardA.SetDutyCycle(100);
	//			std::cout << command << std::endl;
	//		}
	//		else if (command == "move.A.backward")
	//		{
	//			MotorForwardA.Pause(true);
	//			MotorBackwardA.SetDutyCycle(100);
	//			std::cout << command << std::endl;
	//		}
	//		else if (command == "stop.A")
	//		{
	//			MotorBackwardA.Pause(true);
	//			MotorForwardA.Pause(true);
	//			std::cout << command << std::endl;
	//		}
	//		else if (command == "move.B.forward")
	//		{
	//			MotorBackwardB.Pause(true);
	//			MotorBackwardB.SetDutyCycle(100);
	//			std::cout << command << std::endl;
	//		}
	//		else if (command == "move.B.backward")
	//		{
	//			MotorForwardB.Pause(true);
	//			MotorBackwardB.SetDutyCycle(100);
	//			std::cout << command << std::endl;
	//		}

	//		else if (command == "stop.B")
	//		{
	//			MotorBackwardB.Pause(true);
	//			MotorForwardB.Pause(true);
	//			std::cout << command << std::endl;
	//		}
	//		command = "";
	//	}
	//	else
	//	{
	//		std::cout << "enter a command milord: " << std::endl;
	//		std::cin >> command;
	//	}

	//	//std::this_thread::sleep_for(std::chrono::milliseconds(200));

	//}

}