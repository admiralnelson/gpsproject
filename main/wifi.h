#pragma once
#include <string>
#include <vector>
#include <functional>
#include <array>
#include <queue>
#include <mutex>
#include <map>
#include "esp_wifi.h"
#include "cjsoncpp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define DEFAULT_SCAN_LIST_SIZE 3

struct WiFiInfo;
typedef  std::vector<WiFiInfo> ScansResult;
typedef wifi_event_t WiFiEvent;
typedef ip_event_t IPEvent;
typedef wifi_auth_mode_t WiFiAuthType;


typedef std::function<void(ScansResult)> WiFiScanResultCallback;
typedef std::function<void(void*)> WiFiEventCallback;
typedef std::function<void(void*)> IPEventCallback;

enum class WiFiMode
{
    B, G, N
};

struct IPInfo
{
    std::string IPAddress;
    std::string Subnet;
    std::string Gateway;
};

struct WiFiInfo : public JsonSerialisable
{
    WiFiInfo();
    std::string Name;
    std::string Password;
    int8_t Strength;
    WiFiAuthType Auth;
    virtual cjsonpp::JSONObject ToJsonObject() const override;
    virtual bool FromJsonObject(const cjsonpp::JSONObject& Object) noexcept override;
};


/*
    Used internally.
*/
struct WiFiCallbackData
{
    std::string Name;
    WiFiEventCallback Callback;
    WiFiEvent EventId;
};
/*
    Used internally.
*/
struct IPCallbackData
{
    std::string Name;
    IPEventCallback Callback;
    IPEvent EventId;
};

class Wifi
{
public:

    static Wifi& Get()
    {
        static Wifi instance; // Guaranteed to be destroyed.
                                 // Instantiated on first use.
        return instance;
    }

    static std::string Int32ToIPv4String(uint32_t IP);

    bool SetMode(WiFiMode Mode);

    bool RegisterOnIPAcquisiton(std::string& name, IPEventCallback Callback);
    bool RegisterOnConnect(std::string& name, WiFiEventCallback Callback);
    bool RegisterOnDisconnect(std::string& name, WiFiEventCallback Callback);
    bool RegisterWiFiEvent(WiFiEvent EventType, const char* Name, WiFiEventCallback Callback);
    bool RegisterIpEvent(IPEvent EventType, const char* Name, IPEventCallback Callback);

    bool Start(const char* Ssid, const char* Password = nullptr);
    bool Stop();

    bool Reconnect();
    bool StartAP(const char* ApSsid, const char* APPassword = nullptr);
    bool StopAP();

    bool Connect(const std::string& Ssid, const std::string& Password, WiFiAuthType AuthType = WIFI_AUTH_WPA2_PSK);
    bool ConnectFallback();
    bool IsWiFiConnectedToSta() const;
    WiFiInfo GetConnectedStaInfo() const;
    bool IsWiFiAPOn() const;
    //of course Strength data is not used here.
    WiFiInfo GetWiFiAPInfo() const;
    std::string GetWiFiAPPassword() const;
    IPInfo GetWiFiStaNetAddress() const;
    std::string GetWiFiStaMacAddress() const;
    std::string GetWiFiAPMacAddress() const;
    IPInfo GetWiFiAPNetAddress() const;

    bool Scan(ScansResult& Result);
    bool SaveStaConfiguration(const std::string& Path)const;
    bool LoadStaConfiguration(const std::string& Path);
    bool SaveAPConfiguration(const std::string& Path) const;
    bool LoadAPConfiguration(const std::string& Path);    

    bool RemoveIpEvent(const std::string& Name);
    bool RemoveWiFiEvent(const std::string& Name);
    /*
        Used internally. Do not call!
    */
    std::vector<WiFiCallbackData>& GetWiFiCallbackHandlers();
    /*
        Used internally. Do not call!
    */
    std::vector<IPCallbackData>& GetIPCallbackHandlers();
    /*
        Used internally. Do not call!
    */
    std::recursive_mutex& GetMutex();
    /*
        Used internally. Do not call! 
    */
    WiFiEventCallback& GetWiFiMonitor();
    /*
        Used internally. Do not call!
    */
    IPEventCallback& GetIPMonitor();
private:
    WiFiInfo StaConfig;
    WiFiInfo APConfig;

    esp_netif_t* sta_netif;

    std::vector<WiFiCallbackData> ListOfWifiCallbacks;

    std::vector<IPCallbackData> ListOfIPCallbacks;

    std::recursive_mutex Mutex;
    
    WiFiEventCallback WiFiMonitor;
    IPEventCallback IPMonitor;

    bool bConnectedToSta = false;
    bool bAPStarted = false;
private:
    Wifi();
    Wifi(Wifi const&) = delete;
    void operator=(Wifi const&) = delete;
    EventGroupHandle_t s_wifi_event_group;

};