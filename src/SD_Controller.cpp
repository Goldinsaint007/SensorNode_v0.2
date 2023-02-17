#include "SD_Controller.h"

bool SDController::init() {
  if (!SD.begin(2)) {
    Serial.println("SD Card Initialization failed!");
    initialized = false;
  } else {
    initialized = true;
  }
  return initialized;
}

bool SDController::createAndSaveFile(const char* fileName, const char* text) {
  if (!initialized) {
    return false;
  }

  File file = SD.open(fileName, FILE_WRITE);
  if (!file) {
    return false;
  }

  file.println(text);
  file.close();
  return true;
}

char* SDController::readFile(const char* fileName) {
  if (!initialized) {
    return nullptr;
  }

  File file = SD.open(fileName);
  if (!file) {
    return nullptr;
  }

  int fileSize = file.size();
  char* buffer = new char[fileSize + 1];

  int bytesRead = file.readBytes(buffer, fileSize);
  buffer[bytesRead] = '\0';

  file.close();

  return buffer;
}

bool SDController::fileExists(const char* fileName) {
  if (!initialized) {
    return false;
  }

  return SD.exists(fileName);
}

bool SDController::eraseCard() {
  if (!initialized) {
    return false;
  }

  // Call recursive function to delete all files and directories
  if (!deleteDirectory("/")) {
    return false;
  }

  return true;
}

bool SDController::deleteDirectory(String path) {
  File root = SD.open(path);

  if (!root) {
    return false;
  }

  // Delete all files and directories in the current directory
  while (true) {
    File file = root.openNextFile();
    if (!file) {
      break;
    }

    String filePath = path + "/" + file.name();
    if (file.isDirectory()) {
      if (!deleteDirectory(filePath)) {
        file.close();
        root.close();
        return false;
      }
    } else {
      if (!SD.remove(filePath)) {
        file.close();
        root.close();
        return false;
      }
    }

    file.close();
  }

  root.close();

  // Remove the current directory
  if (path != "/") {
    if (!SD.rmdir(path)) {
      return false;
    }
  }

  return true;
}

bool SDController::isCardEmpty() {
  if (!initialized) {
    return false;
  }

  // Check if there are any files or directories in the root directory
  File root = SD.open("/");
  bool isEmpty = true;
  while (true) {
    File file = root.openNextFile();
    if (!file) {
      break;
    }
    isEmpty = false;
    file.close();
  }
  root.close();

  return isEmpty;
}

void SDController::printDirectory(const char *dirname, uint8_t numTabs) {
  File root = SD.open(dirname);

  if (!root) {
    return;
  }

  while (true) {
    File entry = root.openNextFile();

    if (!entry) {
      break;
    }

    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print("\t");
    }

    Serial.print(entry.name());

    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry.name(), numTabs + 1);
    } else {
      Serial.print("\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
  root.close();
}
