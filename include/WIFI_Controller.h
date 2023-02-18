/**
 * File: WifiController.h
 * 
 * @author: Jakobus Johannes Strydom
 * @date: 12/02/2023
 * 
 * Description: This is the header file for the WifiController class, which provides a wrapper for
 * connecting and communicating with a Wi-Fi network.
 **/
#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <Arduino.h>
#include <EEPROM.h>
#include <WIFI_Config.h>
#include <LED_Status.h>

/**
 * WifiController class

 * This class provides a wrapper around the WiFi library to handle
 * connecting to a Wi-Fi network, sending and receiving messages via
 * UDP, and managing the status of the connection.

 * The class also provides methods for saving and loading Wi-Fi
 * credentials, checking if a Wi-Fi connection is established, and
 * checking if a client is connected to the device.

 * A reference to a StatusLED object is passed to the class to provide
 * visual feedback on the status of the Wi-Fi connection.
 **/
class WifiController {
    public:
        WifiController(StatusLED &sLED) : led(sLED) {};
        void init();
        bool connect();
        void checkIncomingClients();
        bool isWiFiConnected();
        bool isClientConnected();
        int sendMessage(String& message);
        int receiveMessage(String& message);
        bool hasCredentials();
        void set_initialized(bool state);
        void saveCredentials(WiFiCredentials credentials);

    private:
        WiFiCredentials loadCredentials();
        void setupUDP();
        void clearCredentials();
        bool get_initialized();
        void broadcastIP(unsigned long timer);
        String WiFiStatusCodeToString(wl_status_t status);

        unsigned long lastPingTime;
        bool connected = false;
        UdpClient client;
        WiFiUDP udp;
        StatusLED &led;
};

#endif