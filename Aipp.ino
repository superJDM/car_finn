
#include"Webserver_spdb.h";
#include""

//Custom WiFi Credentials
#define WIFI_SSID "CMCC-web"
#define WIFI_PASSWD "13164755116"

Webserver *server;
KeyStore* store = KeyStore::getKeyStoreInstance();


void setup() {
    //Load the given configuration details from the SPIFFS
  store->loadJSONConfiguration();
  printl

}

void loop() {
  // put your main code here, to run repeatedly:

}