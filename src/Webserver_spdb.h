#ifndef Webserver_h
#define Webserver_h
#include "BoTESP32SDK.h"
#include "Storage.h"
#include "ControllerService.h"
#include "ConfigurationService.h"
#include "BluetoothService.h"
#include "PairingService.h"
#include "WiFi.h"
#define STARTED 1
#define NOT_STARTED 0

#define  DBG_TEST_CNTR   1

class ConfigurationService;
class Webserver
{
  public:
    static Webserver* getWebserverInstance(bool loadConfig, const char *ssid = NULL,
                          const char *passwd = NULL, const int logLevel = BoT_INFO);
    bool isWiFiConnected();
    bool isServerAvailable();
    void blinkLED();
    void connectWiFi();
    void startServer();
    IPAddress getBoardIP();
  private:
    int port;
    int ledPin;
    int serverStatus;
    int debugLevel;
#ifdef DBG_TEST_CNTR
	int blecntr_lost;
	int blecntr_link;
#endif
    String *WiFi_SSID;
    String *WiFi_Passwd;
    KeyStore *store;
    AsyncWebServer *server;
    ConfigurationService *config;
    static Webserver *webServer;
    BluetoothService *ble;
    bool isDevicePaired();
    Webserver(bool loadConfig, const char *ssid = NULL, const char *passwd = NULL,
                                                   const int logLevel = BoT_INFO);
};

#endif