#ifndef SD_CONTROLLER_H
#define SD_CONTROLLER_H

#include <SD.h>

class SDController {
  public:
    bool init();
    bool createAndSaveFile(const char* fileName, const char* text);
    char* readFile(const char* fileName);
    bool fileExists(const char* fileName);
    bool isInitialized() { return initialized; };
    bool eraseCard();
    bool isCardEmpty();
    void printDirectory(const char *dirname, uint8_t numTabs);
    
  private:
    bool deleteDirectory(String path);
    bool initialized = false;
};

#endif
