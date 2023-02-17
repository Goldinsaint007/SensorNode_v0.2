// IR_Controller.h
#ifndef IR_CONTROLLER_H
#define IR_CONTROLLER_H

#include <IR_Config.h>

class IRController {
    public:
        IRController() {};
        void begin();
        void read(const char* fileName);
        void send(const char* fileName);
        void start();
        void stop();
        bool isReading() { return reading; };
        bool codeReceived = false;

    private:
        void makeText(uint16_t *raw_array, uint16_t length, char* text);
        uint16_t makeArrayFromText(uint16_t* &rawCode, char *text);

        SDController sd;

        // Use turn on the save buffer feature for more complete capture coverage.
        decode_results results;  // Somewhere to store the results
        bool reading = false;
};

#endif  // IR_CONTROLLER_H
