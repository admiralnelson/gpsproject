#include "gps.h"
#include "iostream"
#include "common.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "iomanip"



#define TASK_PRIORITY_UART_EVENTS       10

#define ECHO_TEST_TXD (17)
#define ECHO_TEST_RXD (16)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (2)
#define ECHO_UART_BAUD_RATE     (9600)
#define ECHO_TASK_STACK_SIZE    (2048*5)

static const char* GPS_TAG = "gps service";


static void SerialMonitorTask(void* arg)
{
    Gps* ptrToGps = static_cast<Gps*>(arg);
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t* data = (uint8_t*)malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        // Write data back to the UART
        /*uart_write_bytes(ECHO_UART_PORT_NUM, (const char*)data, len);*/
        if (len) {
            data[len] = '\0';
            std::copy(data, data + BUF_SIZE, ptrToGps->Buffer.begin());
            ptrToGps->Update(len);
        }
    }
}

Gps& Gps::Get()
{
	static Gps instance;
	return instance;
}

void Gps::SetupSerial()
{
    if (this->bSerialIsActive) return;

    this->bSerialIsActive = true;
    
    xTaskCreate(SerialMonitorTask, "uart_echo_task", ECHO_TASK_STACK_SIZE, this, 10, NULL);
    ESP_LOGI(GPS_TAG, "service started");
}

void Gps::SetupGps()
{
    
    this->gpsService->onUpdate += [this]() {
        if (this->IsDebug())
        {
            nmea::GPSService& gps = *(this->gpsService);
            std::cout << (gps.fix.locked() ? "[*] " : "[ ] ") << std::setw(2) << std::setfill(' ') << gps.fix.trackingSatellites << "/" << std::setw(2) << std::setfill(' ') << gps.fix.visibleSatellites << " ";
            std::cout << std::fixed << std::setprecision(2) << std::setw(5) << std::setfill(' ') << gps.fix.almanac.averageSNR() << " dB   ";
            std::cout << std::fixed << std::setprecision(2) << std::setw(6) << std::setfill(' ') << gps.fix.speed << " km/h [" << nmea::GPSFix::travelAngleToCompassDirection(gps.fix.travelAngle, true) << "]  ";
            std::cout << std::fixed << std::setprecision(6) << gps.fix.latitude << "\xF8 " "N, " << gps.fix.longitude << "\xF8 " "E" << "  ";
            std::cout << "+/- " << std::setprecision(1) << gps.fix.horizontalAccuracy() << "m  ";
            std::cout << std::endl;
        }
    };
}

Gps::Gps()
{
    this->nmeaParser.reset(new nmea::NMEAParser());
    this->gpsService.reset(new nmea::GPSService(*(this->nmeaParser.get())));
    ESP_LOGI(GPS_TAG, "initalising buffer");
    this->SetupSerial();
    this->ShowDebug(true);
    this->SetupGps();

    this->longitudeFilter.reset(new MovingAverageFilter(10));
    this->latitudeFilter.reset(new MovingAverageFilter(10));
    this->altitudeFilter.reset(new MovingAverageFilter(10));
}

void Gps::Update(size_t length)
{
    try
    {
        this->nmeaParser->readBuffer((uint8_t*)this->Buffer.data(),length);
    }
    catch(nmea::NMEAParseError & e)
    {
        ESP_LOGE(GPS_TAG, "parse error: %s", e.message.c_str());
    }
    Delay(1000);
}

double Gps::GetLon()
{
    return this->longitudeFilter->Filter(this->gpsService->fix.longitude);
}

double Gps::GetLat()
{
    return this->latitudeFilter->Filter(this->gpsService->fix.latitude);
}

double Gps::GetAltitute()
{
    return this->altitudeFilter->Filter(this->gpsService->fix.altitude);
}

long Gps::GetTime()
{
    return this->gpsService->fix.timestamp.getTime();
}

bool Gps::IsFix()
{
    return this->gpsService->fix.locked();
}

bool Gps::IsDebug()
{
    return this->bShowDebug;
}

void Gps::ShowDebug(bool bIsShowing)
{
    this->bShowDebug = bShowDebug;
}
