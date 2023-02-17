#include <LED_Status.h>

/**
 * The `StatusLED::SetStatus` method is used to set the status of the LED by driving its red, green, and blue components.
 * 
 * @param status : The status to be set. It can be one of the following:
 *                  BOOTED
 *                  BLE_INIT, BLE_CONNECTED, BLE_FAILED, BLE_SEND, BLE_RECEIVE
 *                  WIFI_INIT, WIFI_CONNECTING, WIFI_CONNECTED, WIFI_FAILED, WIFI_SEND, WIFI_RECEIVE
 *                  UDP_BROADCAST_SENT, UDP_BROADCAST_FAILED, UDP_INVALID_ARGUMENT, UDP_UNKNOWN_ERROR
 *                  DEFAULT
 * 
 * @param alsoDrive : If set to `true`, the drive method will be called after the status is set.
 * 
 * The method sets the `flash` variable to `false` and then performs different operations based on the value of `status`.
 * It sets the status of the LED by calling `setActiveLED` with different parameters, such as the color of the LED, its on/off state, and its brightness.
 * If the `alsoDrive` parameter is `true`, it calls the `drive` method at the end.
 */
void StatusLED::SetStatus(LEDStatus status, bool alsoDrive, unsigned long wait) {
    flash = false;
    this->status = status;

    switch (this->status) {
/*** Other *** Other *** Other *** Othe *** Other *** Other *** Other *** Other *** Other *** Other ***/
        case BOOTED:
            setActiveLED('r', true, 2, 200);
            drive();
            setActiveLED('g', true, 2, 200);
            drive();
            setActiveLED('b', true, 2, 200);
            drive();
            break;

/*** BLE *** BLE *** BLE *** BLE *** BLE *** BLE *** BLE *** BLE *** BLE *** BLE *** BLE *** BLE ***/
        case BLE_INIT:
            setActiveLED('b', true, 2);
            break;
        case BLE_CONNECTED:
            setActiveLED('b');
            break;
        case BLE_DISCONNECTED:
            setActiveLED('r', true, 2);
            break;
        case BLE_FAILED:
            setActiveLED('r', true, 3);
            break;
        case BLE_SEND:
            setActiveLED('g', true, 4);
            break;
        case BLE_RECEIVE:
            setActiveLED('g', true, 4);
            break;

/*** WI-FI *** WI-FI *** WI-FI *** WI-FI *** WI-FI *** WI-FI *** WI-FI *** WI-FI *** WI-FI *** WI-FI ***/
        case WIFI_INIT:
            setActiveLED('g', true, 1);
            break;
        case WIFI_CONNECTING:
            setActiveLED(((activeLED == 'g') ? 'a' : 'g'), true, 1);
            break;
        case WIFI_CONNECTED:
            setActiveLED('g', false);
            break;
        case WIFI_FAILED:
            setActiveLED('r', true, 3);
            break;
        case WIFI_SEND:
            setActiveLED('g', true, 5);
            break;
        case WIFI_RECEIVE:
            setActiveLED('g', true, 3);
            break;
        case WIFI_CONNECTION_LOST:
            setActiveLED('r', true, 10, 2000, 10);
            break;

/* UDP *** UDP *** UDP *** UDP *** UDP *** UDP *** UDP *** UDP *** UDP *** UDP *** UDP *** UDP *** UDP ***/
        case UDP_BROADCAST_SENT:
            setActiveLED('b', true, 2, 100);
            break;
        case UDP_BROADCAST_FAILED:
            setActiveLED('r', true, 3);
            break;
        case UDP_INVALID_ARGUMENT:
            setActiveLED('r', true, 5);
            break;
        case UDP_UNKNOWN_ERROR:
            setActiveLED('r', true, 10);
            break;

/*** DEFAULT *** DEFAULT *** DEFAULT *** DEFAULT *** DEFAULT *** DEFAULT *** DEFAULT *** DEFAULT *** DEFAULT ***/
        default:
            Serial.println("Error: Invalid status code in StatusLED::SetStatus");
            setActiveLED('r', true, 10);
            break;
    }

    if(alsoDrive) {
        drive();
        if(wait != 0) {
            delay(wait);
        }
    }
}

/**
 * The `StatusLED::setActiveLED` method is used to set the active LED, its flashing state, and flash speed.
 * 
 * @param color : The color of the LED to be set as active. Can be 'r', 'g', or 'b' for red, green, and blue, respectively.
 * 
 * @param flash : A flag indicating whether the LED should flash or not. 
 *                If set to `true`, the LED will flash; otherwise, it will be turned on continuously.
 * 
 * @param flashCount : The number of times the LED should flash.
 * 
 * @param duration : The duration of the flashing, in milliseconds.
 * 
 * @param flashSpeed : The flash speed of the LED, in terms of the number of calls to the `drive` method.
 *                     A higher value means a slower flash rate, and a lower value means a faster flash rate.
 * 
 * The method sets the value of the `activeLED` variable to the `color` parameter, the `flash` variable to the `flash` parameter, 
 * the `flashCount` variable to the `flashCount` parameter, the `duration` variable to the `duration` parameter,
 * and the `flashRate` variable to the `flashSpeed` parameter.
 */
void StatusLED::setActiveLED(char color, bool flash, int flashCount, int duration, int flashRate) {
    activeLED = color;              // Set the value of activeLED to color
    this->flash = flash;            // Set the value of flash to flash
    this->flashCount = flashCount;  // Set the value of flashCount to flashCount
    this->duration = duration;      // Set the value of duration to duration
    this->flashRate = flashRate;    // Set the value of flashRate to flashRate
}

/**
 * This function is used to switch off the LED. It sets the digital output of the red, green, and blue pins to LOW, which turns off the LED.
 * The LED is represented by the `StatusLED` class and controlled by the `redPin`, `greenPin`, and `bluePin` class attributes.
 */
void StatusLED::switchOff() {
    digitalWrite(redPin, LOW);      // Set the digital output of the red pin to LOW
    digitalWrite(greenPin, LOW);    // Set the digital output of the green pin to LOW
    digitalWrite(bluePin, LOW);     // Set the digital output of the blue pin to LOW
}

/**
 * The `drive` method controls the behavior of the LED.
 * 
 * If `flash` is set to `true`, the LED will flash at the specified `flashRate` speed.
 * If `flash` is set to `false`, the LED will remain turned on continuously in the specified color.
 * The method uses the `activeLED` character variable to determine the color of the LED, and the `digitalWrite` function
 * to turn the LED on or off.
 */
void StatusLED::drive() {
    // Calculate the current time in milliseconds.
    int currentTime = millis();
    // Store the start time for use in calculations later.
    int startTime = currentTime;

    // If `flash` is true, turn the LED on and off repeatedly.
    if (flash) {
        // Calculate the amount of time the LED should be on and off.
        int onTime = (duration / flashRate) / 2;
        int offTime = onTime;
        // Repeat the on/off cycle `flashCount` times.
        while (flashCount > 0) {
            // Recalculate the current time.
            currentTime = millis();
            // Calculate the elapsed time since the start of the cycle.
            int elapsedTime = currentTime - startTime;
            // If the elapsed time is within the on period, turn on the LED.
            if (elapsedTime % (duration / flashRate) < onTime) {
                // Turn on the appropriate LED based on the value of `activeLED`.
                switch (activeLED) {
                    case 'a':
                        digitalWrite(redPin, HIGH);
                        digitalWrite(greenPin, HIGH);
                        break;
                    case 'r':
                        digitalWrite(redPin, HIGH);
                        break;
                    case 'g':
                        digitalWrite(greenPin, HIGH);
                        break;
                    case 'b':
                        digitalWrite(bluePin, HIGH);
                        break;
                    default:
                        break;
                }
                // Wait for the on period to end.
                delay(onTime);
                // Turn off all the LEDs.
                switchOff();
                // Decrement the number of remaining cycles.
                flashCount--;
                // Wait for the off period to end.
                delay(offTime);
            }
        }
    } else {
        // If `flash` is false, turn off all the LEDs and turn on the appropriate LED based on `activeLED`.
        switchOff();
        switch (activeLED) {
            case 'r':
                digitalWrite(redPin, HIGH);
                break;
            case 'g':
                digitalWrite(greenPin, HIGH);
                break;
            case 'b':
                digitalWrite(bluePin, HIGH);
                break;
            default:
                break;
        }
    }
    #ifdef _DEBUG
    Serial.println("Current Status: " + String(status) + " | Current LED: " + String(activeLED));
    Serial.println("Should Flash: " + String(flash) + " | Flash Count: " + String(flashCount));
    Serial.println("Duration: " + String(duration) + " | Flash Rate: " + String(flashRate));
    #endif
}
