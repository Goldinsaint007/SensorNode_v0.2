#include <BLE_Controller.h>

#ifdef EASYDEBUG
  bool _ssid = false;
  bool _pass = false;
#endif 

bool r = false;
bool s = false;

void ble_receive(String uuid) {
  r = true;
}

void ble_send(String uuid) {
  s = true;
}

typedef void (*_recieve)(String uuid);
typedef void (*_send)(String uuid);

class MyServerCallbacks : public BLEServerCallbacks {
  public:
    bool connected = false;

    void onConnect(BLEServer *pServer) {
      connected = true;
    }

    void onDisconnect(BLEServer *pServer) {
      connected = false;
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  public:
    _recieve recv;
    _send snd;

    void setNotifyCharacteristic(BLECharacteristic *pCharacteristic) {
      pNotifyCharacteristic = pCharacteristic;
    }

    // Callback function for when a write event occurs on the characteristic
    void onWrite(BLECharacteristic *pCharacteristic) {
      String replyValue = ""; 
      String uid = pCharacteristic->getUUID().toString().c_str();
      if(uid == CHARACTERISTIC_UUID_SSID) {
        replyValue = "SSID ";
      }
      if(uid == CHARACTERISTIC_UUID_PASSWORD) {
        replyValue = "PASS ";
      }

      // Get the string value of the written data
      std::string value = pCharacteristic->getValue();
      
      recv(uid);

      // If the value is not empty, the data was received successfully
      if (!value.empty()) {
          // Send a notification indicating that the data was received successfully
          pNotifyCharacteristic->setValue(replyValue + "Good\0");
          pNotifyCharacteristic->notify();
          snd(uid);
      } else {
          // Send a notification indicating that the data was not received successfully
          pNotifyCharacteristic->setValue(replyValue + "Bad\0");
          pNotifyCharacteristic->notify();
          snd(uid);
      }
    }

  private:
    // Characteristic for notifications
    BLECharacteristic *pNotifyCharacteristic;
};

/**
 * @brief Initialize the Bluetooth Low Energy (BLE) server and advertise the Wi-Fi configuration service.
 * 
 * This function initializes the BLE server using the BLEDevice class. It creates a BLE service using the service UUID and
 * creates two characteristics for the SSID and password with write properties.
 * 
 * The function sets callbacks for the characteristics and starts both the service and advertising.
 * 
 * Finally, the function sets the callbacks for the BLE server using the MyServerCallbacks object.
 **/
void BLEController::init() {
  led.SetStatus(BLE_INIT);

#ifdef EASYDEBUG
  delay(100);
  Serial.println("BLE - intitalizing...");
#endif 

  mscb = new MyServerCallbacks();
  ssidCallback = new MyCharacteristicCallbacks();
  ssidCallback->recv = ble_receive;
  ssidCallback->snd = ble_send;
  passwordCallback = new MyCharacteristicCallbacks();
  passwordCallback->recv = ble_receive;
  passwordCallback->snd = ble_send;

#ifdef EASYDEBUG
  delay(100);
  Serial.print("BLE - Start with name :");
  Serial.println(DEVICE_NAME);
#endif

  // Initialize the BLE device with the name "DEVICE_NAME" from "BLE_Config"
  BLEDevice::init(DEVICE_NAME);

#ifdef EASYDEBUG
  delay(100);
  Serial.println("BLE - Creating server...");
#endif 

  // Create the BLE server
  pServer = BLEDevice::createServer();

#ifdef EASYDEBUG
  delay(100);
  Serial.print("BLE - Creating service with UUID :");
  Serial.println(SERVICE_UUID);
#endif

  // Create a service with the service UUID
  BLEService *pService = pServer->createService(SERVICE_UUID);

#ifdef EASYDEBUG
  delay(100);
  Serial.println("BLE - Creating characteristic for (WIFI SSID) with write property...");
  Serial.print("BLE - UUID :");
  Serial.println(CHARACTERISTIC_UUID_SSID);
#endif

  // Create a characteristic for the SSID with write property and set its callbacks using the MyCharacteristicCallbacks object
  pSsidCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_SSID,
      NIMBLE_PROPERTY::WRITE);
  pSsidCharacteristic->setCallbacks(ssidCallback);

#ifdef EASYDEBUG
  delay(100);
  Serial.println("BLE - Creating characteristic for (WIFI password) with write property...");
  Serial.print("BLE - UUID :");
  Serial.println(CHARACTERISTIC_UUID_PASSWORD);
#endif

  // Create a characteristic for the password with write property and set its callbacks using the MyCharacteristicCallbacks object
  pPasswordCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_PASSWORD,
      NIMBLE_PROPERTY::WRITE);
  pPasswordCharacteristic->setCallbacks(passwordCallback);

#ifdef EASYDEBUG
  delay(100);
  Serial.println("BLE - Creating characteristic for (Notifications) with notify property...");
  Serial.print("BLE - UUID :");
  Serial.println(CHARACTERISTIC_UUID_NOTIFY);
#endif

  // Create a characteristic for notifications with notify property
  pNotifyCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_NOTIFY,
      NIMBLE_PROPERTY::NOTIFY);
  ssidCallback->setNotifyCharacteristic(pNotifyCharacteristic);
  passwordCallback->setNotifyCharacteristic(pNotifyCharacteristic);

#ifdef EASYDEBUG
  delay(100);
  Serial.println("BLE - Start the service...");
#endif 

  // Start the service and advertising
  pService->start();

#ifdef EASYDEBUG
  delay(100);
  Serial.println("BLE - Start advertising...");
#endif 

  pServer->getAdvertising()->start();

#ifdef EASYDEBUG
  delay(100);
  Serial.println("BLE - Set the callbacks for the server...");
#endif 

  // Set the callbacks for the BLE server using the MyServerCallbacks object
  pServer->setCallbacks(mscb);

#ifdef EASYDEBUG
  Serial.println("BLE - intitalization done...");
#endif
}

/**
 * @brief Retrieve Wi-Fi credentials from a BLE client and store them in the WifiCredentials object.
 * 
 * The function uses a loop to check if a BLE client has connected. If a client has connected, the function retrieves
 * the SSID and password from the characteristic values and stores them in the WifiCredentials object using the 
 * saveCredentials() function. If the Wi-Fi credentials have been initialized, the function restarts the ESP.
 * 
 * The function also includes a timeout of 2 minutes. If no client has made a connection within 2 minutes, the ESP will
 * restart. If the client disconnects before sending all the data, the timeout will reset.
 * 
 * @param wifiController The WifiCredentials object used to store the Wi-Fi credentials.
 */
void BLEController::GetWiFi() {
#ifdef EASYDEBUG
  delay(100);
  Serial.println("BLE - Start looking for credentials...");
#endif 

  // Store the start time in milliseconds
  unsigned long startTime = millis();

  // Loop until the SSID and password have been retrieved from the characteristic values
  while (ssid.empty() || password.empty()) {
    connected();
    if(r) {
      led.SetStatus(BLE_RECEIVE);
      r = false;
    }
    if(s) {
      led.SetStatus(BLE_SEND);
      s = false;
    }

    // Check if a client has connected
    if (mscb->connected) {
      // Reset the start time when a client connects
      startTime = millis();

      // Retrieve the SSID and password from the characteristic values
      ssid = pSsidCharacteristic->getValue();

#ifdef EASYDEBUG
    if(ssid != "" && !_ssid) {
      delay(100);
      Serial.println("BLE - SSID :" + String(ssid.c_str()));
      _ssid = true;
    }
#endif 

      password = pPasswordCharacteristic->getValue();

#ifdef EASYDEBUG
    if(password != "" && !_pass) {
      delay(100);
      Serial.println("BLE - Password :" + String(password.c_str()));
      _pass = true;
    }
#endif 
    }

    // Check if 2 minutes have passed since the start time
    if (millis() - startTime >= 2 * 60 * 1000) {
#ifdef EASYDEBUG
      delay(100);
      Serial.println("BLE - Looking for connection timed out rebooting...");
      delay(1000);
#endif 
      // ESP Restart
      ESP.restart();
    }
  }

  // Store the Wi-Fi credentials in the WifiCredentials object
  wifi.saveCredentials(WiFiCredentials(ssid, password));

  // Check if the Wi-Fi credentials have been initialized
  if (wifi.hasCredentials()) {
#ifdef EASYDEBUG
    delay(100);
    Serial.println("BLE - WIFI credentials recived and stored rebooting...");
    delay(1000);
#endif 
    // ESP Restart
    ESP.restart();
  }
}

void BLEController::connected() {
  if(!_connected && mscb->connected) {
    led.SetStatus(BLE_CONNECTED);
    _connected = true;
#ifdef EASYDEBUG
    delay(100);
    Serial.println("BLE - Client connected waiting for SSID and Password...");
#endif 
  }
  if(_connected && !mscb->connected) {
    led.SetStatus(BLE_DISCONNECTED);
    _connected = false;
#ifdef EASYDEBUG
    delay(100);
    Serial.println("BLE - Client disconnected...");
#endif 
  }
}
