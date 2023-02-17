#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <WiFi.h>

static constexpr int EEPROM_SIZE     = 512; // size of the EEPROM memory
static constexpr int SSID_ADDRESS    = 0; // start address of the Wi-Fi SSID string
static constexpr int SSID_LENGTH     = 32; // maximum length of the Wi-Fi SSID string
static constexpr int PASS_ADDRESS    = SSID_ADDRESS + SSID_LENGTH; // start address of the Wi-Fi password string
static constexpr int PASS_LENGTH     = 64; // maximum length of the Wi-Fi password string
static constexpr int INIT_ADDRESS    = PASS_ADDRESS + PASS_LENGTH; // address to store the initialization state of Wi-Fi credentials
static constexpr int LOCAL_PORT      = 8181;
static constexpr char PASS_PHRASE[]  = "abc\0";

/**
 * UdpClient struct

 * Structure to store the IP address and port number of a UDP client.

 * The ip field holds the IP address of the client and the port field holds the port number 
 * that the client is connected to.
 **/
struct UdpClient {
    IPAddress ip;
    int port;
};

/**
 * WiFiCredentials struct

 * This struct contains the SSID and password information for connecting to a Wi-Fi network.

 * The struct has two members:
 * * ssid: the name of the Wi-Fi network
 * * password: the password for the Wi-Fi network

 * The struct also provides a default constructor and an overloaded constructor for initializing the
 * ssid and password members.
 **/
struct WiFiCredentials {
    std::string ssid;
    std::string password;
    WiFiCredentials() {};
    WiFiCredentials(std::string _ssid, std::string _pass) : ssid(_ssid), password(_pass) {};
};

#endif