#include "persistence.h"
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

const char* PERSISTENCE_TAG = "PERSISTENCE";

Persistence& Persistence::Get()
{
	static Persistence instance;
	return instance;
}

void Persistence::WriteInt(const std::string& name, int integer)
{
    esp_err_t err;
    err = nvs_set_i32(this->NVSHandleNr, name.c_str(), integer);
    if (err != ESP_OK)
    {
        ESP_LOGE(PERSISTENCE_TAG, "failed to write variable %s, reason: %s", name.c_str(), esp_err_to_name(err));
    }
    err = nvs_commit(this->NVSHandleNr);
    if (err != ESP_OK)
    {
        ESP_LOGE(PERSISTENCE_TAG, "failed to commit, reason: %s", esp_err_to_name(err));
    }
}

Persistence::IntReturn Persistence::GetInt(const std::string& name)
{
    esp_err_t err;
    int32_t result = 0; // value will default to 0, if not set yet in NVS
    err = nvs_get_i32(this->NVSHandleNr, name.c_str(), &result);
    IntReturn retVal;
    retVal.Result = result;
    retVal.Existed = true;
    switch (err) 
    {
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGW(PERSISTENCE_TAG, "The value %s is not initialized yet!", name.c_str());
            retVal.Existed = false;
            break;
        case ESP_OK:
            break;
        default:
            ESP_LOGE(PERSISTENCE_TAG, "Error (%s) reading!\n", esp_err_to_name(err));
            retVal.Existed = false;
    }

    return retVal;
}

Persistence::Persistence()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
        ESP_LOGE(PERSISTENCE_TAG, "trying to reflash NVS parition");
    }
    ESP_ERROR_CHECK(err);

    // Open
    ESP_LOGI(PERSISTENCE_TAG, "Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    this->NVSHandleNr = my_handle;
    ESP_LOGI(PERSISTENCE_TAG, "Handle is now %d", (int) this->NVSHandleNr);

    if (err != ESP_OK) 
    {
        ESP_LOGE(PERSISTENCE_TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
}
