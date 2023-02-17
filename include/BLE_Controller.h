#ifndef BLE_CONTROLLER_H_
#define BLE_CONTROLLER_H_

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <LED_Status.h>
#include <WIFI_Controller.h>
#include <BLE_Config.h>

class MyServerCallbacks;
class MyCharacteristicCallbacks;

/**
 * BLEController class

 * This class provides a wrapper around the BLE library to handle connecting to a 
 * BLE network, sending and receiving data.

 * The class also provides methods for managing the BLE connection, handling incoming
 * connections and notifying changes to connected devices.

 * A reference to a WifiController and StatusLED object is passed to the class to
 * provide integration with the wifi and LED status functions.
 **/
class BLEController {
    public:
        BLEController(WifiController _wifi, StatusLED &sLED) : wifi(_wifi), led(sLED) {};
        void init();
        void GetWiFi();

    private:
        std::string ssid;
        std::string password;
        BLEServer* pServer = NULL;
        BLECharacteristic* pSsidCharacteristic = NULL;
        BLECharacteristic* pPasswordCharacteristic = NULL;
        BLECharacteristic* pNotifyCharacteristic = NULL;
        MyServerCallbacks *mscb;
        MyCharacteristicCallbacks *ssidCallback;
        MyCharacteristicCallbacks *passwordCallback;
        WifiController &wifi;
        void connected();
        bool _connected = false;
        StatusLED &led;
};

#endif /* BLE_CONTROLLER_H_ */
