#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>
#include <EasyDebug.h>

//#define _DEBUG

/*
 * LEDStatus is an enumeration that represents the various statuses of the device's LED.
 * It includes status codes related to Bluetooth Low Energy (BLE) and Wi-Fi connectivity.
 *
 * BOOTED: The device has successfully booted and is ready for use
 *
 * BLE_INIT: The BLE module is initializing
 * BLE_CONNECTED: A BLE connection has been established
 * BLE_FAILED: A BLE connection has failed
 * BLE_SEND: Data is being sent over BLE
 * BLE_RECEIVE: Data is being received over BLE
 *
 * WIFI_INIT: The Wi-Fi module is initializing
 * WIFI_CONNECTING: The device is connecting to a Wi-Fi network
 * WIFI_CONNECTED: The device is connected to a Wi-Fi network
 * WIFI_FAILED: The Wi-Fi connection has failed
 * WIFI_SEND: Data is being sent over Wi-Fi
 * WIFI_RECEIVE: Data is being received over Wi-Fi
 */
enum LEDStatus {
    // The device has successfully booted and is ready for use
    BOOTED = 0,

    // Bluetooth Low Energy (BLE) related status codes
    BLE_INIT = 10,              // The BLE module is initializing
    BLE_CONNECTED = 11,         // A BLE connection has been established
    BLE_DISCONNECTED = 12,      // A BLE connection has been lost
    BLE_FAILED = 13,            // A BLE connection has failed
    BLE_SEND = 14,              // Data is being sent over BLE
    BLE_RECEIVE = 15,           // Data is being received over BLE

    // Wi-Fi related status codes
    WIFI_INIT = 20,             // The Wi-Fi module is initializing
    WIFI_CONNECTING = 21,       // The device is connecting to a Wi-Fi network
    WIFI_CONNECTED = 22,        // The device is connected to a Wi-Fi network
    WIFI_FAILED = 23,           // The Wi-Fi connection has failed
    WIFI_SEND = 24,             // Data is being sent over Wi-Fi
    WIFI_RECEIVE = 25,          // Data is being received over Wi-Fi
    WIFI_CONNECTION_LOST = 26,  // Indicates that the WiFi connection has been lost.

    // UDP related status codes
    UDP_BROADCAST_SENT = 30,    // The broadcast packet was sent successfully
    UDP_BROADCAST_FAILED = 31,  // The broadcast packet failed to be sent
    UDP_INVALID_ARGUMENT = 32,  // An invalid argument was provided to the method
    UDP_UNKNOWN_ERROR = 33      // An unknown error occurred during the broadcast process
};

class StatusLED {
    public:
        StatusLED() {};
        StatusLED(int red, int green, int blue) : redPin(red), greenPin(green), bluePin(blue) {
            pinMode(red, OUTPUT);
            pinMode(green, OUTPUT);
            pinMode(blue, OUTPUT);
        };
        void setActiveLED(char color, bool flash = false, int flashCount = 1, int duration = 1000, int flashRate = 1);
        void SetStatus(LEDStatus status, bool alsoDrive = true, unsigned long wait = 0);
        void switchOff();
        void drive();
        LEDStatus getStatus() { return this->status; };

    private:
        int redPin;
        int greenPin;
        int bluePin;
        char activeLED;
        bool flash;
        int duration;
        int flashRate;
        int flashCount;
        LEDStatus status;
};

#endif
