#include "Webserver_spdb.h"
#ifndef DEBUG_DISABLED
  RemoteDebug Debug;
#endif

Webserver* Webserver :: webServer;

Webserver* Webserver :: getWebserverInstance(bool loadConfig, const char *ssid,
                          const char *passwd, const int logLevel){
  if(webServer == NULL){
    webServer = new Webserver(loadConfig, ssid, passwd, logLevel);
    LOG("\nWebserver :: getWebserverInstance: Instantiated AsyncWebServer Instance...");
  }

  return webServer;
}

Webserver :: Webserver(bool loadConfig, const char *ssid, const char *passwd, const int logLevel){
  ledPin = 2;
  port = 3001;
#ifdef DBG_TEST_CNTR
  blecntr_lost = 0;
  blecntr_link = 0;
#endif
  WiFi_SSID = NULL;
  WiFi_Passwd = NULL;
  server = NULL;
  store = NULL;
  config = NULL;
  ble = NULL;
  debugLevel = logLevel;
  serverStatus = NOT_STARTED;
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);

  if(loadConfig == false){
    WiFi_SSID = new String(ssid);
    WiFi_Passwd = new String(passwd);
  }
  store = KeyStore::getKeyStoreInstance();
}

void Webserver :: connectWiFi(){
  if(WiFi_SSID == NULL || WiFi_Passwd == NULL){
    store->loadJSONConfiguration();
    store->initializeEEPROM();
    WiFi_SSID = new String(store->getWiFiSSID());
    WiFi_Passwd = new String(store->getWiFiPasswd());
  }

  LOG("\nWebserver :: connectWiFi: Connecting to WiFi SSID: %s", WiFi_SSID->c_str());
  if(!isWiFiConnected()){
    WiFi.mode(WIFI_STA);
    WiFi.begin(WiFi_SSID->c_str(), WiFi_Passwd->c_str());

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        LOG("\nWebserver :: connectWiFi: Trying to Connect to WiFi SSID: %s failed, rebooting the board", WiFi_SSID->c_str());
        delay(5000);
        ESP.restart();
    }
  }
    LOG("\nWebserver :: connectWiFi: Board Connected to WiFi SSID: %s, assigned IP: %s", WiFi_SSID->c_str(), (getBoardIP().toString()).c_str());
    blinkLED();
    //Remote Debug Setup if DEBUG ENABLED
    #ifndef DEBUG_DISABLED
      //Initialise RemoteDebug instance based on provided log level
      switch(debugLevel){
        case BoT_DEBUG: Debug.begin("BoT-ESP-32",Debug.DEBUG); break;
        case BoT_INFO: Debug.begin("BoT-ESP-32",Debug.INFO); break;
        case BoT_WARNING: Debug.begin("BoT-ESP-32",Debug.WARNING); break;
        case BoT_ERROR: Debug.begin("BoT-ESP-32",Debug.ERROR); break;
        default: Debug.begin("BoT-ESP-32",Debug.INFO); break;
      }
      //Set required properties for Debug
      Debug.setResetCmdEnabled(true);
      Debug.setSerialEnabled(true);
      Debug.showProfiler(true);
      Debug.showColors(true);
    #endif
 }

IPAddress Webserver :: getBoardIP(){
  if(isWiFiConnected() == true){
    return WiFi.localIP();
  }
}

void Webserver::blinkLED()
{
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
}

bool Webserver :: isWiFiConnected(){
   return (WiFi.status() == WL_CONNECTED)?true:false;
}

bool Webserver :: isDevicePaired(){
  //Check pairing status for the device
  PairingService* ps = new PairingService();
  String* psResponse = ps->getPairingStatus();
  delete ps;

  debugD("+-- rsp  %s.\n", psResponse->c_str());
  return true;

  if((psResponse->indexOf("true")) != -1)
    return true;
  else
    return false;
}

void Webserver :: startServer(){
   if(isWiFiConnected() == true){
     debugD("\nWebserver :: startServer: Starting the Async Webserver...");

     server = new AsyncWebServer(port);

     config = new ConfigurationService();

     ble = new BluetoothService();

     server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        //request->send(200, "text/plain", "Banking of Things ESP-32 SDK Webserver");
        AsyncJsonResponse * response = new AsyncJsonResponse();
        response->addHeader("Server","ESP-32 Dev Module Async Web Server");
        JsonObject& root = response->getRoot()
        root["actionsEndPoint"]  = "/actions";
        root["pairingEndPoint"]  = "/pairing";
        root["activateEndPoint"] = "/activate";
        root["qrCodeEndPoint"]   = "/qrcode";
        root["actionEndPoint"]   = "/action?actionID=`actionID-value`";
        response->setLength();
        request->send(response);
      });

      server->on("/actions", HTTP_GET, [](AsyncWebServerRequest *request){
         ControllerService cs;
         cs.getActions(request);
      });

      server->on("/action", HTTP_GET, [](AsyncWebServerRequest *request){
         ControllerService cs;
         cs.postAction(request);
      });

      server->on("/pairing", HTTP_GET, [](AsyncWebServerRequest *request){
         ControllerService cs;
         cs.pairDevice(request);
      });

      server->on("/activate", HTTP_GET, [](AsyncWebServerRequest *request){
         ControllerService cs;
         cs.activateDevice(request);
      });

      server->on("/qrcode", HTTP_GET, [](AsyncWebServerRequest *request){
         ControllerService cs;
         cs.getQRCode(request);
      });

      server->begin();
      serverStatus = STARTED;
      debugI("\nWebserver :: startServer: BoT Async Webserver started on ESP-32 board at port: %d, \nAccessible using the URL: http://%s:%d/", port,(getBoardIP().toString()).c_str(),port);

      //Device is already paired, then device initialization is skipped
      //Otherwise waits till device gets paired using FINN APP either by
      //BLE client connects and key exchanges happen or by QR Code
      //if(isDevicePaired()){
      if(0){
        debugI("\nWebserver :: startServer: Device is already paired, checking device's state is valid or not");
        //Below situation occurs when the same device is switched between Multipair and Singlepair
        //Reset Device State and Initialize
        if((!store->isDeviceMultipair() && store->getDeviceState() == DEVICE_MULTIPAIR) ||
           (store->isDeviceMultipair() && store->getDeviceState() != DEVICE_MULTIPAIR))
        {
          debugI("\nWebserver :: startServer: Invalid device state, initializing as new device");
          store->resetDeviceState();
          store->resetQRCodeStatus();
          config->initialize();
          config->configureDevice();
        }
        else
          debugI("\nWebserver :: startServer: Valid device state, no need to initialize and configure");
      }
      else {
        debugI("\nWebserver :: startServer: Device is not paired yet, needs initialization");
        config->initialize();
        debugD("\nWebserver :: startServer: Free Heap before BLE Init: %u", ESP.getFreeHeap());
        ble->initializeBLE();
        bool bleClientConnected = false;
        unsigned long startTime = millis();
        //Wait till device gets paired from FINN APP for 2 mins through BLE
        do {
          delay(2000);
          bleClientConnected = ble->isBLEClientConnected();
          if(!bleClientConnected)
#ifdef DBG_TEST_CNTR
          {
    		  blecntr_lost++;
    		  if (blecntr_lost % 30 == 0) {
    			debugI("\nWebserver :: startServer: Waiting for BLE Client connect (%d)...", blecntr_lost);
    		  }
          }
#else
		  debugI("\nWebserver :: startServer: Waiting for BLE Client to connect...");
#endif
          else
            debugI("\nWebserver :: startServer: BLE Client connected to BLE Server...");
        }while(!bleClientConnected && (millis() - startTime)<(2*60*1000));

        //Wait till BLE Client disconnects from BLE Server
        while(bleClientConnected){
#ifdef DBG_TEST_CNTR
          blecntr_link++;
          if (blecntr_link % 30 == 0) {
          	debugI("\nWebserver :: startServer: Waiting for BLE Client disconnect (%d)...", blecntr_link);
          }
#else
          debugI("\nWebserver :: startServer: Waiting for BLE Client to disconnect from BLE Server");
#endif
          bleClientConnected = ble->isBLEClientConnected();
          delay(2000);
        }

        //Release memory used by BLE Service once BLE Client gets disconnected
        if(!bleClientConnected) ble->deInitializeBLE();
        debugD("\nWebserver :: startServer: Free Heap after BLE deInit: %u", ESP.getFreeHeap());

        //If device does not get paired through BLE,
        //wait now till it gets paired through QR Code if QR Code is available
        if(store->isQRCodeGeneratedandSaved()){
          while(!isDevicePaired()){
            debugI("\nWebserver :: startServer: Waiting for device pairing through QR Code...\n");
            delay(2000);
          }
          //Proceed with configuring the device
          config->configureDevice();
        }
        else
          debugW("\nWebserver :: startServer: QR Code not available for the device, try again!!!");

     }
   }
   else {
     LOG("\nWebserver :: startServer: ESP-32 board not connected to WiFi Network");
   }
}

bool Webserver :: isServerAvailable(){
   if(isWiFiConnected() && serverStatus == STARTED)
      return true;
   else {
      digitalWrite(ledPin, LOW);
      return false;
  }
}
