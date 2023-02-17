/**
 * WifiController.cpp
 * Implementation file for the WifiController class.
 * 
 * The class provides a wrapper around the WiFi library to handle
 * connecting to a Wi-Fi network, sending and receiving messages via
 * UDP, and managing the status of the connection.
 * 
 * The methods in this file define the functionality of the class.
 * 
 * @author: Jakobus Johannes Strydom
 * @date: 12/02/2023
 **/

#include <WIFI_Controller.h>

#pragma region WifiController::init()
/**
 * @brief Initialize the WifiController instance
 *
 * This function initializes the WifiController instance by setting the delay,
 * initializing the EEPROM, and setting the initialization status.
 *
 * @param none
 * @return none
 */
void WifiController::init() {
    delay(10); // Delay for 10 milliseconds
    EEPROM.begin(EEPROM_SIZE); // Begin the EEPROM with the specified size
    led.SetStatus(WIFI_INIT, true, 500); // Set the WIFI_INIT status of the LED
}
#pragma endregion

#pragma region WifiController::sendMessage()
/**
 * @brief Sends a message to a specific client over the UDP protocol
 * 
 * This function sends the message specified in the input to a client identified by its IP address and port. The message
 * is sent using the UDP protocol.
 * 
 * @param message A reference to a String object that contains the message to be sent
 * @return An integer indicating the number of bytes sent. Returns 0 if the sending process fails.
 */
int WifiController::sendMessage(String& message) {
    int result = 0;
    // Begin sending a packet to the specified client's IP address and port
    if (udp.beginPacket(client.ip, client.port)) {
        // Write the message to the packet
        result = udp.write((uint8_t*) message.c_str(), message.length());
        // End the packet and send it
        if (!udp.endPacket()) {
            result = 0; // Return 0 if the packet was not successfully sent
        }
    }
    return result;
}
#pragma endregion

#pragma region WifiController::receiveMessage()
/**
 * @brief Receives a message sent over the UDP protocol
 * 
 * This function receives a message sent over the UDP protocol and stores it in the `receivedMsg` input. The function 
 * first checks the size of the incoming packet and then, if it's not empty, retrieves the sender's IP address. The
 * function then verifies if the last octet of the sender's IP is not 255 and that it's different from the local IP
 * address. If the verification is successful, the function reads the packet and stores it in the `receivedMsg` input,
 * returning the size of the packet.
 * 
 * @param receivedMsg A reference to a String object that will store the received message
 * @return The size of the received packet. Returns 0 if the packet is empty or if the sender's IP address is not valid.
 */
int WifiController::receiveMessage(String& receivedMsg) {
    int packetSize = udp.parsePacket(); // Get the size of the incoming packet
    if (packetSize) { // If the packet is not empty
        IPAddress senderIP = udp.remoteIP(); // Retrieve the sender's IP address
        // Check if the last octet of the sender IP is not 255 and that it's different from the local IP address
        if (senderIP[3] != 255 && senderIP[3] != WiFi.localIP()[3]) {
            char buffer[packetSize + 1]; // Create a buffer to store the received message
            int len = udp.read((unsigned char*) buffer, packetSize); // Read the packet
            buffer[len] = '\0'; // Terminate the string
            receivedMsg = String(buffer); // Store the received message in the input
            return packetSize; // Return the size of the packet
        }
    }
    return 0; // Return 0 if the packet is empty or if the sender's IP address is not valid
}

#pragma endregion

#pragma region WifiController::checkIncomingClients()
/**
 * @brief Checks for incoming clients
 * 
 * This function checks for incoming clients by calling the `receiveMessage()` function and storing the received
 * message in a `String` object. If the function returns a non-zero value, indicating that a message was received,
 * the function then checks if the received message matches the `PASS_PHRASE`. If the match is successful, the function
 * stores the sender's IP address and port in the `client` object and sets the `connected` flag to `true`. The function
 * then sends a message back to the client using the `sendMessage()` function and updates the value of `lastPingTime`
 * to the current time in milliseconds.
 * 
 * The function also calls the `broadcastIP()` function with a time interval of 1000 milliseconds.
 */
void WifiController::checkIncomingClients() {
    String receivedMsg; // String object to store the received message
    int packetSize = receiveMessage(receivedMsg); // Get the size of the received packet
    if (packetSize != 0) { // If the packet is not empty
        Serial.println("Received message: " + receivedMsg + " and the pass phrase is : " + PASS_PHRASE);
        if (receivedMsg == PASS_PHRASE) { // Check if the received message matches the `PASS_PHRASE`
            client.ip = udp.remoteIP(); // Store the sender's IP address in the `client` object
            client.port = udp.remotePort(); // Store the sender's port in the `client` object
            connected = true; // Set the `connected` flag to `true`
            Serial.println("client ip: " + client.ip.toString() + " client port: " + String(client.port));
            String msg;
            msg += "Hello";
            sendMessage(msg); // Send a message back to the client
            lastPingTime = millis(); // Update the value of `lastPingTime`
        }
    }

    // Call the `broadcastIP()` function with a time interval of 1000 milliseconds
    broadcastIP(1000);
}

#pragma endregion

#pragma region WifiController::broadcastIP()
/**
 * @brief Broadcasts the local IP address over UDP.
 *
 * This function sends the local IP address in the form of a string over UDP
 * to the broadcast address on the specified local port. It will only send a
 * broadcast if the time since the last broadcast is greater than the timer's
 * value.
 *
 * @param timer The minimum time in milliseconds between broadcasts.
 */
void WifiController::broadcastIP(unsigned long timer) {
    // store the time of the last broadcast
    static unsigned long lastBroadcastTime = 0;

    // check if it has been more than timer's value in milliseconds since the last broadcast
    if (millis() - lastBroadcastTime >= timer) {
        // calculate the broadcast address by setting the last octet of the local IP address to 255
        IPAddress broadcastAddress = WiFi.localIP();
        broadcastAddress[3] = 255;

        // create a char array to store the local IP address in the form of a string
        char localIPMessage[32];
        sprintf(localIPMessage, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);

        // send the local IP message over UDP to the broadcast address on the specified local port
        udp.beginPacket(broadcastAddress, LOCAL_PORT);
        udp.write((uint8_t*) localIPMessage, strlen(localIPMessage));
        int status = udp.endPacket();
        if (status == 1) {
            // Packet was successfully sent
            lastBroadcastTime = millis();
            led.SetStatus(UDP_BROADCAST_SENT, true);
        } else {
            // Failed to send the packet
            int writeError = udp.getWriteError();
            if (writeError != 0) {
                led.SetStatus(UDP_BROADCAST_FAILED, true);
            } else {
                led.SetStatus(UDP_UNKNOWN_ERROR, true);
            }
        }
    }
}
#pragma endregion

#pragma region WifiController::isClientConnected()
/**
 * @brief Check if a client is connected.
 * 
 * The function sends a "ping" message to the client and waits for a response within 1000 milliseconds. 
 * If the response is "pong", the function returns `true` indicating that the client is connected. 
 * If it has been more than 1 minute since the last "ping" message was sent, a new "ping" message is sent to the client.
 * If no response is received, the function returns `false` indicating that the client is not connected.
 * 
 * @return `true` if the client is connected, `false` otherwise.
 */
bool WifiController::isClientConnected() {
    if (connected) {
        unsigned long currentTime = millis();
        // Check if it has been more than 60000 milliseconds (1 minute) since the last ping
        if (currentTime - lastPingTime >= 60000) {  
            String message = "ping";
            // Send a ping message to the client
            int bytesSent = sendMessage(message);
            Serial.println("ping sent to ip: " + client.ip.toString());
            unsigned long startTime = millis();
            // Wait for 1000 milliseconds for a response
            while (millis() - startTime < 1000) {  
                int bytesReceived = receiveMessage(message);
                // Check if a response was received and if it was "pong"
                if (bytesReceived > 0 && message == "pong") {
                    connected = true;
                    break;
                }
                connected = false;
            }
            // If no response or the response was not "pong", the client is considered disconnected
            if (!connected) {
                Serial.println("Lost connection");
                return false;
            }
            // Update the last ping time
            lastPingTime = currentTime;
        }
    }
    // Output the connection status
    Serial.println((connected) ? "true" : "false");
    return connected;
}
#pragma endregion

#pragma region WifiController::saveCredentials()
/**
 * Method to save WiFi credentials in EEPROM.
 * 
 * This method takes the input WiFi credentials and truncates the SSID and password to the maximum
 * length defined by the constants SSID_LENGTH and PASS_LENGTH. 
 * 
 * The truncated SSID and password are then written to EEPROM, and a null character is 
 * added at the end of each string to indicate the end of the string. 
 * 
 * The "initialized" flag is set to indicate that the credentials have been saved
 * to EEPROM. Finally, the changes to EEPROM are committed.
 * 
 * @param cred The WiFi credentials (SSID and password) to be saved.
 **/
void WifiController::saveCredentials(WiFiCredentials cred) {
    // Truncate the input SSID and password to the maximum length defined by SSID_LENGTH and PASS_LENGTH.
    cred.ssid = cred.ssid.substr(0, SSID_LENGTH - 1);
    cred.password = cred.password.substr(0, PASS_LENGTH - 1);

    // Write the truncated SSID to EEPROM
    for (int i = 0; i < cred.ssid.length(); i++) {
        EEPROM.write(SSID_ADDRESS + i, cred.ssid[i]);
    }
    // Add a null character at the end of the SSID string in EEPROM
    EEPROM.write(SSID_ADDRESS + cred.ssid.length(), 0);
    
    // Write the truncated password to EEPROM
    for (int i = 0; i < cred.password.length(); i++) {
        EEPROM.write(PASS_ADDRESS + i, cred.password[i]);
    }
    // Add a null character at the end of the password string in EEPROM
    EEPROM.write(PASS_ADDRESS + cred.password.length(), 0);
    
    // Set the "initialized" flag to indicate that the credentials have been saved to EEPROM
    set_initialized(true);

    // Commit the changes to EEPROM
    EEPROM.commit();
}
#pragma endregion

#pragma region WifiController::loadCredentials()
/**
 * @brief Load WiFi credentials from EEPROM memory
 * 
 * This method reads the saved SSID and password from EEPROM memory and returns it as a WiFiController object.
 * 
 * The method reads characters from EEPROM memory one by one until a null character is encountered, which indicates the end of the string.
 * 
 * The read characters are appended to the corresponding SSID and password strings.
 * 
 * @return WiFiCredentials object containing the loaded SSID and password.
 **/
WiFiCredentials WifiController::loadCredentials() {
    WiFiCredentials cred;
    // Read SSID from EEPROM
    for (int i = 0; i < SSID_LENGTH; i++) {
        char c = char(EEPROM.read(SSID_ADDRESS + i));
        if (c == 0) { 
            break; // End of SSID string
        }
        cred.ssid += c; // Add character to SSID string
    }

    // Read password from EEPROM
    for (int i = 0; i < PASS_LENGTH; i++) {
        char c = char(EEPROM.read(PASS_ADDRESS + i));
        if (c == 0) {
            break; // End of password string
        }
        cred.password += c; // Add character to password string
    }
    return cred;
}
#pragma endregion

#pragma region WifiController::hasCredentials()
/**
 * WifiController::hasCredentials()
 * 
 * Returns a boolean indicating whether the WiFi credentials have been saved to the EEPROM memory.
 * 
 * The method first checks the value of the "initialized" flag, which indicates whether the credentials
 * have been saved.
 * 
 * If the flag is false, the method returns false immediately.
 * 
 * If the flag is true, the method loads the saved SSID and password from the EEPROM memory using the loadCredentials() method.
 * 
 * Finally, the method returns true if both the SSID and password have a length greater than zero.
 * 
 * @return A boolean indicating whether the WiFi credentials have been saved to the EEPROM memory.
 **/
bool WifiController::hasCredentials() {
    // Check if the WiFi credentials have been saved by checking the value of the initialized flag.
    if (!get_initialized()) { return false; }

    // Load the saved SSID and password from the EEPROM memory.
    WiFiCredentials cred = loadCredentials();

    // Return true if both the SSID and password have a length greater than zero.
    return cred.ssid.length() > 0 && cred.password.length() > 0;
}
#pragma endregion

#pragma region WifiController::clearCredentials()
/**
 * This method clears the saved WiFi credentials from the EEPROM memory.
 * 
 * The steps taken to clear the credentials include:
 *  * Looping through the EEPROM memory from the start address of the SSID to the end address of the
 *    SSID and setting each byte to zero.
 * 
 *  * Looping through the EEPROM memory from the start address of the password to the end address of the
 *    password and setting each byte to zero.
 * 
 * Setting the initialized flag to false to indicate that the WiFi credentials have not been saved.
 * 
 * Committing the changes to the EEPROM memory.
 **/
void WifiController::clearCredentials() {
    // Loop through the EEPROM memory from the start address of the SSID to the end address of the SSID
    // and set each byte to zero.
    for (int i = SSID_ADDRESS; i < SSID_ADDRESS + SSID_LENGTH; i++) {
        EEPROM.write(i, 0);
    }

    // Loop through the EEPROM memory from the start address of the password to the end address of the password
    // and set each byte to zero.
    for (int i = PASS_ADDRESS; i < PASS_ADDRESS + PASS_LENGTH; i++) {
        EEPROM.write(i, 0);
    }

    // Set the initialized flag to false to indicate that the WiFi credentials have not been saved.
    set_initialized(false);

    // Commit the changes to the EEPROM memory.
    EEPROM.commit();
}
#pragma endregion

#pragma region WifiController::set_initialized()
/**
 * set_initialized() - method to set the initialized flag to indicate if WiFi credentials have been saved.
 * 
 * This method writes the initialized state (represented by the boolean value "init") to the EEPROM memory.
 * 
 * The initialized flag is used to indicate if WiFi credentials have been saved and is stored at the INIT_ADDRESS location in the EEPROM.
 * 
 * The changes are committed to the EEPROM memory after writing the initialized state.
 * 
 * @param init: boolean value representing the initialized state (true if WiFi credentials have been saved, false otherwise)
 **/
void WifiController::set_initialized(bool init) {
    // Write the initialized state to the EEPROM
    EEPROM.write(INIT_ADDRESS, init);

    // Commit the write to the EEPROM
    EEPROM.commit();
}
#pragma endregion

#pragma region WifiController::get_initialized()
/**
 * get_initialized() method is used to retrieve the value of the "initialized" flag that indicates
 * if the WiFi credentials have been saved to the EEPROM memory.
 * 
 * @return Returns the value of the "initialized" flag as a boolean, true if the credentials have been saved,
 * and false otherwise.
 **/
bool WifiController::get_initialized() {
    return EEPROM.read(INIT_ADDRESS);
}
#pragma endregion

#pragma region WifiController::connect()
/**
 * connect - Connect to WiFi network using saved credentials.
 *  
 * This method connects to a WiFi network using the credentials that were previously saved.
 * 
 * If no saved credentials are found, a message is printed and the method returns.
 * 
 * The method first calls the hasCredentials method to check if credentials have been saved.
 * 
 * If there are no saved credentials, the method returns.
 * 
 * If there are saved credentials, the method calls the loadCredentials method to retrieve
 * the SSID and password.
 * 
 * The WiFi connection is then started using these credentials.
 * 
 * The connection is monitored, and if it fails to connect within 10 seconds, a message is printed,
 * the LED status is updated, and the ESP32 device is restarted.
 * 
 * If the connection is successful, a message is printed with the device's
 * IP address, and the LED status is updated.
 * 
 * Finally, the method calls the setupUDP method to setup the UDP connection.
 **/
bool WifiController::connect() {

    // Check if credentials have been saved
    if (!hasCredentials()) {
        // If no credentials have been saved, return
        Serial.println("No saved credentials found");
        led.SetStatus(WIFI_FAILED, true, 1000);
        return false;
    }

    // Load SSID and password from saved credentials
    WiFiCredentials cred = loadCredentials();

    // Start the WiFi connection using the loaded credentials
    Serial.println("Connecting to WiFi network " + String(cred.ssid.c_str()) + "...");
    WiFi.begin(cred.ssid.c_str(), cred.password.c_str());
    delay(100);

    // Wait for the connection to be established
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED) {
        if (timeout++ >= 10 || WiFi.status() == WL_CONNECT_FAILED) {
            // If the connection fails, return
            Serial.println("Failed to connect to " + String(cred.ssid.c_str()) + " with a status of :" + WiFiStatusCodeToString(WiFi.status()));
            led.SetStatus(WIFI_FAILED, true, 1000);
            Serial.println("rebooting in 1 sec...");
            delay(1000);
            WiFi.disconnect();
            ESP.restart();
        }
        led.SetStatus(WIFI_CONNECTING);
    }

    // Connection successful
    Serial.println("Successfully connected to " + String(cred.ssid.c_str()));
    Serial.println("IP Address : " + WiFi.localIP().toString());

    led.SetStatus(WIFI_CONNECTED);

    // Setup UDP connection
    setupUDP();

    return true;
}
#pragma endregion

#pragma region WifiController::setupUDP()
/**
 * @brief Sets up the UDP connection.
 * 
 * This method starts the UDP connection on the specified local port and
 * prints a message to the Serial console to indicate that the connection has been established.
 **/
void WifiController::setupUDP() {
    // start the UDP connection on the specified local port
    udp.begin(LOCAL_PORT);

    // print a message to the Serial console to indicate that the connection has been established
    Serial.println("UDP connection established on port " + String(LOCAL_PORT));
}
#pragma endregion

#pragma region WifiController::isWiFiConnected()
/**
 * isWiFiConnected
 * Check if the WiFi connection is established
 * 
 * @return True if the WiFi connection is established, False otherwise
 **/
bool WifiController::isWiFiConnected() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(String(led.getStatus()) + " " + String(WIFI_CONNECTED));
        if(led.getStatus() != WIFI_CONNECTED) {
            led.SetStatus(WIFI_CONNECTED, true, 1000);
        }
        return true;
    } else {
        led.SetStatus(WIFI_CONNECTION_LOST);
        return false;
    }
}
#pragma endregion

String WifiController::WiFiStatusCodeToString(wl_status_t status) {
    switch (status) {
    case WL_NO_SHIELD:
        return "WL_NO_SHIELD";
        break;
    case WL_IDLE_STATUS:
        return "WL_IDLE_STATUS";
        break;
    case WL_NO_SSID_AVAIL:
        return "WL_NO_SSID_AVAIL";
        break;
    case WL_SCAN_COMPLETED:
        return "WL_SCAN_COMPLETED";
        break;
    case WL_CONNECTED:
        return "WL_CONNECTED";
        break;
    case WL_CONNECT_FAILED:
        return "WL_CONNECT_FAILED";
        break;
    case WL_CONNECTION_LOST:
        return "WL_CONNECTION_LOST";
        break;
    case WL_DISCONNECTED:
        return "WL_DISCONNECTED";
        break;
    
    default:
        return "NULL";
        break;
    }
}
