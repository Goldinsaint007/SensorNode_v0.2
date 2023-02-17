#include <Arduino.h>
#include <LED_driver.h>
#include <WIFI_Controller.h>
#include <BLE_Controller.h>
#include <IR_Controller.h>

WifiController wifi(SLED);
BLEController bt(wifi, SLED);
IRController ir;

void setup() {
#ifdef EASYDEBUG
  Serial.begin(115200);
  while (!Serial) {
    // code 
  }
  Serial.println("ESP32 Booted");
#endif
  SLED.SetStatus(BOOTED, true, 1000); // Set the BOOTED status of the LED

  wifi.init();
  //wifi.set_initialized(false);

  // Check if the Wi-Fi credentials are saved in EEPROM
  if (wifi.hasCredentials()) {
    ir.begin();
    wifi.connect();
  } else {
    bt.init();
    bt.GetWiFi();
  }
}

void loop() {
  if(wifi.isWiFiConnected()) {
    if(!wifi.isClientConnected()) {
      wifi.checkIncomingClients();
    } else {
      String data;
      if(wifi.receiveMessage(data) > 0) {
        Serial.println(data);
        data = "OK";
        wifi.sendMessage(data);
      }
    }
  }
}