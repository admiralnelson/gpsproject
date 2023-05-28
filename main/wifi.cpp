#include "esp_log.h"
#include "wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/dns.h"
#include "lwip/sockets.h"
#include <sstream>
#include <cstring>
#include "filelogger.h"
#include "filesystem.h"
#include "esp_netif.h"

#define SOFT_AP_IP_ADDRESS_1 192
#define SOFT_AP_IP_ADDRESS_2 168
#define SOFT_AP_IP_ADDRESS_3 2
#define SOFT_AP_IP_ADDRESS_4 50

#define SOFT_AP_GW_ADDRESS_1 192
#define SOFT_AP_GW_ADDRESS_2 168
#define SOFT_AP_GW_ADDRESS_3 1
#define SOFT_AP_GW_ADDRESS_4 1

#define SOFT_AP_NM_ADDRESS_1 255
#define SOFT_AP_NM_ADDRESS_2 255
#define SOFT_AP_NM_ADDRESS_3 255
#define SOFT_AP_NM_ADDRESS_4 0

#define DNS_ADDRESS_1 8
#define DNS_ADDRESS_2 8
#define DNS_ADDRESS_3 8
#define DNS_ADDRESS_4 8

#define TAG "wifi"

void EventHandler(void* arg, esp_event_base_t EventBase, int32_t EventId, void* event_data)
{
    assert(arg != nullptr);
    Wifi* WiFiInstance = (Wifi*)arg;

    if (WiFiInstance->GetMutex().try_lock())
    {
        if (EventBase == IP_EVENT)
        {
            std::vector<IPCallbackData>& Data = WiFiInstance->GetIPCallbackHandlers();
            for (auto const& d : Data)
            {
                if (d.EventId == (IPEvent)EventId)
                {
                    d.Callback(event_data);
                    ESP_LOGI(TAG, "ran IP callback with name '%s'", d.Name.c_str());
                }
            }
            if ((IPEvent)EventId == IP_EVENT_STA_GOT_IP)
            {
                WiFiInstance->GetIPMonitor()(event_data);
            }
        }
        else if (EventBase == WIFI_EVENT)
        {
            std::vector<WiFiCallbackData>& Data = WiFiInstance->GetWiFiCallbackHandlers();
            for (auto const& d : Data)
            {
                if (d.EventId == (WiFiEvent)EventId)
                {
                    d.Callback(event_data);
                    ESP_LOGI(TAG, "ran WiFi callback with name '%s'", d.Name.c_str());
                }
            }
            if ((WiFiEvent)EventId == WIFI_EVENT_STA_DISCONNECTED)
            {
                WiFiInstance->GetWiFiMonitor()(event_data);
            }
        }
        WiFiInstance->GetMutex().unlock();
    }
    else
    {
        ESP_LOGW(TAG, "cannot process callback, mutex was locked! event was %s event id %d",EventBase, (int) EventId);
    }

   
}

void _handle_sta_got_ip(void* arg, esp_event_base_t event_base,
    int event_id, void* event_data)
{
    ip_event_got_ip_t const* const event = (ip_event_got_ip_t*)event_data;
    ESP_LOGI(TAG, "IP addr " IPSTR, IP2STR(&event->ip_info.ip));
    
    // this would be a great place to start OTA Update
    //xTaskCreate(&ota_update_task, "ota_update_task", 8192, NULL, 5, NULL);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data)
{

}

Wifi::Wifi()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL,
        NULL));

    ESP_LOGI(TAG, "initialised");

}

std::string Wifi::Int32ToIPv4String(uint32_t IP)
{
    std::stringstream ss;
    unsigned int  ipAddress = IP;
    unsigned char octet[4] = { 0,0,0,0 };

    for (int i = 0; i < 4; i++)
    {
        octet[i] = (ipAddress >> (i * 8)) & 0xFF;
    }

    ss << (int)octet[0] << "." << (int)octet[1] << "." << (int)octet[2] << "." << (int)octet[3];
    return ss.str();
}

bool Wifi::RegisterOnIPAcquisiton(std::string& name, IPEventCallback Callback)
{
    if (Mutex.try_lock())
    {
        IPCallbackData Data {
             name,
             Callback,
             IP_EVENT_STA_GOT_IP
        };
        ListOfIPCallbacks.push_back(Data);
        Mutex.unlock();
        return true;
    }
    else
    {
        ESP_LOGW(TAG, "mutex, fail to execute this function %s", __FUNCTION__);
        return false;
    }
}

bool Wifi::RegisterOnConnect(std::string& name, WiFiEventCallback Callback)
{
    if (Mutex.try_lock())
    {

        Mutex.unlock();
        return true;
    }
    else
    {
        ESP_LOGW(TAG, "mutex, fail to execute this function %s", __FUNCTION__);
        return false;
    }
}

bool Wifi::RegisterOnDisconnect(std::string& name, WiFiEventCallback Callback)
{
    if (Mutex.try_lock())
    {

        Mutex.unlock();
        return true;
    }
    else
    {
        ESP_LOGW(TAG, "mutex, fail to execute this function %s", __FUNCTION__);
        return false;
    }
}

bool Wifi::RegisterWiFiEvent(WiFiEvent Event, const char* Name, WiFiEventCallback Callback)
{
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, Event, &EventHandler, this));
    WiFiCallbackData Data{
        Name,
        Callback,
        Event
    };
    ListOfWifiCallbacks.push_back(Data);
    return true;
}

bool Wifi::RegisterIpEvent(IPEvent EventType, const char* Name, IPEventCallback Callback)
{
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, EventType, &EventHandler, this));
    IPCallbackData Data{
        Name,
        Callback,
        EventType
    };
    ListOfIPCallbacks.push_back(Data);
    return true;
}

bool Wifi::Start(const char* Ssid, const char* Password)
{
    if (Mutex.try_lock())
    {

        Mutex.unlock();
        return true;
    }
    else
    {
        ESP_LOGW(TAG, "mutex, fail to execute this function %s", __FUNCTION__);
        return false;
    }
}

bool Wifi::Reconnect()
{
    if (Mutex.try_lock())
    {

        Mutex.unlock();
        return true;
    }
    else
    {
        ESP_LOGW(TAG, "mutex, fail to execute this function %s", __FUNCTION__);
        return false;
    }
}

bool Wifi::StartAP(const char* ApSsid, const char* APPassword)
{
    wifi_config_t WifiConfig;
    memset(&WifiConfig, 0, sizeof(WifiConfig));
    WifiConfig.ap.max_connection = 4;
    strcpy((char*)WifiConfig.ap.ssid, ApSsid);
    if (APPassword != nullptr)
    {
        strcpy((char*)WifiConfig.ap.password, APPassword);
        WifiConfig.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
        APConfig.Password = APPassword;
    }
    else
    {
        WifiConfig.ap.authmode = WIFI_AUTH_OPEN;
        APConfig.Password = "";
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &WifiConfig));
    std::string msg = "Enabled local access point with name " + std::string((const char*) WifiConfig.ap.ssid);
    FileLogger::Get().WriteLogInfo(TAG, msg);
    APConfig.Name = ApSsid;

    ESP_ERROR_CHECK(esp_wifi_start());

    Mutex.unlock();
    return true;

}


bool Wifi::StopAP()
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    FileLogger::Get().WriteLogInfo(TAG, "Disabled local access point");
    return true;
}


bool Wifi::Scan(ScansResult& Result)
{
    if (Mutex.try_lock())
    {
        esp_wifi_scan_start(NULL, true);
        Mutex.unlock();

        ESP_LOGW(TAG, "scanning...");
        uint16_t TotalAps = DEFAULT_SCAN_LIST_SIZE;
        wifi_ap_record_t ApInfos[DEFAULT_SCAN_LIST_SIZE];
        uint16_t TotalFound = 0;
        std::memset(ApInfos, 0, sizeof(ApInfos));
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&TotalAps, ApInfos));
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&TotalFound));
        ESP_LOGI(TAG, "Total APs scanned = %u", TotalFound);
        for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < TotalFound); i++)
        {
            ESP_LOGI(TAG, "SSID \t\t%s", ApInfos[i].ssid);
            ESP_LOGI(TAG, "RSSI \t\t%d", ApInfos[i].rssi);
            ESP_LOGI(TAG, "Channel \t\t%d\n", ApInfos[i].primary);
            WiFiInfo AP;
            AP.Name = (char*)ApInfos[i].ssid;
            AP.Strength = ApInfos[i].rssi;
            AP.Auth = ApInfos[i].authmode;
            Result.push_back(AP);
        }
        return true;
    }
    else
    {
        ESP_LOGW(TAG, "mutex, fail to execute this function %s", __FUNCTION__);
        return false;
    }
}

bool Wifi::SaveStaConfiguration(const std::string& Path) const
{
    ESP_LOGI(TAG, "saving sta config to file");
    return FileSystem::Get().WriteFile(Path.c_str(), StaConfig.ToJson());
}

bool Wifi::LoadStaConfiguration(const std::string& Path) 
{
    std::string JsonData;
    bool bSuccess = FileSystem::Get().LoadFile(Path.c_str(), JsonData);
    bSuccess &= StaConfig.FromJson(JsonData);
    if (bSuccess)
    {
        bSuccess &= Connect(StaConfig.Name, StaConfig.Password, StaConfig.Auth);
    }
    ESP_LOGI(TAG, "loading and connecting sta config from file");
    return bSuccess;
}

bool Wifi::SaveAPConfiguration(const std::string& Path) const
{
    ESP_LOGI(TAG, "saving ap config to file");
    return FileSystem::Get().WriteFile(Path.c_str(), APConfig.ToJson());
}

bool Wifi::LoadAPConfiguration(const std::string& Path)
{
    std::string JsonData;
    bool bSuccess = FileSystem::Get().LoadFile(Path.c_str(), JsonData);
    bSuccess &= APConfig.FromJson(JsonData);
    if (bSuccess)
    {
        bSuccess &= StartAP(APConfig.Name.c_str(), APConfig.Password.c_str());
    }
    ESP_LOGI(TAG, "loading and starting AP config from file");
    return bSuccess;
}

bool Wifi::Connect(const std::string& Ssid, const std::string& Password, WiFiAuthType AuthType)
{

    wifi_config_t wifiConfig;
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    wifiConfig.sta.threshold.authmode = AuthType;
    wifiConfig.sta.pmf_cfg = {
        .capable = true,
        .required = false
    },

    strcpy((char*)wifiConfig.sta.ssid, Ssid.c_str());
    strcpy((char*)wifiConfig.sta.password, Password.c_str());

    ESP_LOGI(TAG, "attempted to connect %s", wifiConfig.sta.ssid);

    esp_err_t result = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    esp_err_t result2 = esp_wifi_connect();
    ESP_ERROR_CHECK(result);
    ESP_ERROR_CHECK(result2);
   
    //monitor if WiFi disconnected or no
    WiFiMonitor = [this](void* Result)
    {
        bConnectedToSta = false;
        std::string Msg = "Disconnected from access point/WLAN station";
        FileLogger::Get().WriteLogInfo(TAG, Msg);
    };
    IPMonitor = [this, Ssid, Password, AuthType](void* Data)
    {
        bConnectedToSta = true;
        std::string WifiName = GetConnectedStaInfo().Name;
        std::string IPAddr = GetWiFiStaNetAddress().IPAddress;
        std::string Msg = "Connceted to wifi: " + WifiName + " IP: "+ IPAddr;
        FileLogger::Get().WriteLogInfo(TAG, Msg);
        StaConfig.Name = Ssid;
        StaConfig.Password = Password;
        StaConfig.Auth = AuthType;
    };

    return result == ESP_OK && result2 == ESP_OK;
}

bool Wifi::ConnectFallback()
{
    if (StaConfig.Name != "")
    {
        ESP_LOGI(TAG, "attempt to fallback to %s", StaConfig.Name.data());
        return Connect(StaConfig.Name, StaConfig.Password, StaConfig.Auth);
    }
    else
    {
        return false;
    }
}

bool Wifi::IsWiFiConnectedToSta() const
{
    return bConnectedToSta;
}

WiFiInfo Wifi::GetConnectedStaInfo() const
{
    wifi_ap_record_t Status;
    esp_wifi_sta_get_ap_info(&Status);
    WiFiInfo Info;
    Info.Name = (char*)Status.ssid;
    Info.Auth = Status.authmode;
    Info.Strength = Status.rssi;
    return Info;
}

bool Wifi::IsWiFiAPOn() const
{
    wifi_mode_t Mode;
    esp_wifi_get_mode(&Mode);

    return (Mode == WIFI_MODE_APSTA) || (Mode == WIFI_MODE_AP);
}

WiFiInfo Wifi::GetWiFiAPInfo() const
{
    wifi_config_t Config;
    esp_wifi_get_config(WIFI_IF_AP, &Config);
    WiFiInfo Info;
    Info.Name = (char*)Config.ap.ssid;
    Info.Auth = Config.ap.authmode;
    return Info;
}

std::string Wifi::GetWiFiAPPassword() const
{
    return APConfig.Password;
}

IPInfo Wifi::GetWiFiStaNetAddress() const
{
    esp_netif_ip_info_t Info;
    esp_netif_get_ip_info(sta_netif, &Info);

    return { 
        Int32ToIPv4String(Info.ip.addr), 
        Int32ToIPv4String(Info.netmask.addr), 
        Int32ToIPv4String(Info.gw.addr) };
}

std::string Wifi::GetWiFiStaMacAddress() const
{
    std::stringstream ss;
    uint8_t Mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, Mac);

    ss << std::hex << (int)Mac[0] 
        << ":" << std::hex << (int)Mac[1]
        << ":" << std::hex << (int)Mac[2]
        << ":" << std::hex << (int)Mac[3]
        << ":" << std::hex << (int)Mac[4]
        << ":" << std::hex << (int)Mac[5]
        << ":" << std::hex << (int)Mac[6];

    return ss.str();
}

std::string Wifi::GetWiFiAPMacAddress() const
{
    std::stringstream ss;
    uint8_t Mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, Mac);

    ss << std::hex << (int)Mac[0]
        << ":" << std::hex << (int)Mac[1]
        << ":" << std::hex << (int)Mac[2]
        << ":" << std::hex << (int)Mac[3]
        << ":" << std::hex << (int)Mac[4]
        << ":" << std::hex << (int)Mac[5]
        << ":" << std::hex << (int)Mac[6];

    return ss.str();
}

IPInfo Wifi::GetWiFiAPNetAddress() const
{
    uint32_t subnet;
    uint32_t ip;

    ip = LWIP_MAKEU32(
        SOFT_AP_IP_ADDRESS_1,
        SOFT_AP_IP_ADDRESS_2,
        SOFT_AP_IP_ADDRESS_3,
        SOFT_AP_IP_ADDRESS_4);

    subnet = LWIP_MAKEU32(
        SOFT_AP_NM_ADDRESS_1,
        SOFT_AP_NM_ADDRESS_2,
        SOFT_AP_NM_ADDRESS_3,
        SOFT_AP_NM_ADDRESS_4);

    return {
        Int32ToIPv4String(__builtin_bswap32(ip)),
        Int32ToIPv4String(__builtin_bswap32(subnet)),
        "" };
}

bool Wifi::Stop()
{
    esp_wifi_disconnect();
    return true;
}

bool Wifi::RemoveIpEvent(const std::string& Name)
{
    Mutex.lock();
    int len = ListOfIPCallbacks.size();
    int index = 0;
    bool success = false;
    for (index = 0; index < len; index++)
    {
        if (ListOfIPCallbacks[index].Name == Name)
        {
            success = true;
            break;
        }
    }
    if (success)
    {
        ListOfIPCallbacks.erase(ListOfIPCallbacks.begin() + index);
        ESP_LOGI(TAG, "removed Ip event with name %s", Name.data());
    }
    Mutex.unlock();
    return success;
}

bool Wifi::RemoveWiFiEvent(const std::string& Name)
{
    Mutex.lock();
    int len = ListOfWifiCallbacks.size();
    int index = 0;
    bool success = false;
    for (index = 0; index < len; index++)
    {
        if (ListOfWifiCallbacks[index].Name == Name)
        {
            success = true;
            break;
        }
    }
    if (success)
    {
        ListOfWifiCallbacks.erase(ListOfWifiCallbacks.begin() + index);
        ESP_LOGI(TAG, "removed wifi event with name %s", Name.data());
    }
    Mutex.unlock();
    return success;
}

std::vector<WiFiCallbackData>& Wifi::GetWiFiCallbackHandlers()
{
    return this->ListOfWifiCallbacks;
}

std::vector<IPCallbackData>& Wifi::GetIPCallbackHandlers()
{
    return this->ListOfIPCallbacks;
}

std::recursive_mutex& Wifi::GetMutex()
{
    return Mutex;
}

WiFiEventCallback& Wifi::GetWiFiMonitor()
{
    return WiFiMonitor;
}

IPEventCallback& Wifi::GetIPMonitor()
{
    return IPMonitor;
}

WiFiInfo::WiFiInfo()
{
    Strength = 0;
    Auth = WIFI_AUTH_OPEN;
}

cjsonpp::JSONObject WiFiInfo::ToJsonObject() const
{
    cjsonpp::JSONObject Object;
    Object.set("Name", Name);
    Object.set("Password", Password);
    Object.set<int>("Strength", Strength);
    Object.set<int>("Auth", (int)Auth);

    return Object;
}

bool WiFiInfo::FromJsonObject(const cjsonpp::JSONObject& Object) noexcept
{
    using namespace cjsonpp;
    try
    {
        Name = Object.get<std::string>("Name");
        Password = Object.get<std::string>("Password");
        if (Object.has("Auth"))
        {
            Auth = (WiFiAuthType)Object.get<int>("Auth");
        }
        return true;
    }
    catch (const JSONError& e)
    {
        ESP_LOGE(TAG, "%s", e.what());
        return false;
    }
}