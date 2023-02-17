// IR_Controller.cpp
#include <IR_Controller.h>

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);

// The IR transmitter.
IRsend irsend(kIrLedPin);

void IRController::begin() {
  // Perform a low level sanity checks that the compiler performs bit field
  // packing as we expect and Endianness is as we expect.
  assert(irutils::lowLevelSanityCheck() == 0);

#ifdef EASYDEBUG
  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#endif
#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH

  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.

  if(!sd.init()) {
#ifdef EASYDEBUG
    Serial.println("SD Card faild to initialize...!");
#endif
  }
}

void IRController::read(const char* fileName) {
  // Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    // Convert the results into an array suitable for sendRaw().
    // resultToRawArray() allocates the memory we need for the array.
    uint16_t *raw_array = resultToRawArray(&results);
    // Find out how many elements are in the array.
    uint16_t length = getCorrectedRawLength(&results);

    char text[3072];
    makeText(raw_array, length, text);

#ifdef EASYDEBUG
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);

    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow)
        Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);

    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");

    // Display the tolerance percentage if it has been change from the default.
    if (kTolerancePercentage != kTolerance)
        Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);

    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));

    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length())
        Serial.println(D_STR_MESGDESC ": " + description);

    yield(); // Feed the WDT as the text output can take a while to print.

#if LEGACY_TIMING_INFO
    // Output legacy RAW timing info of the result.
    Serial.println(resultToTimingInfo(&results));
    yield();  // Feed the WDT (again)
#endif  // LEGACY_TIMING_INFO

    Serial.print("Test output : ");
    Serial.println(text);

    // Output the results as source code
    Serial.println(resultToSourceCode(&results));

    Serial.println();    // Blank line between entries
#endif

    sd.createAndSaveFile(("/" + std::string(fileName)).c_str(), text);
    codeReceived = true;
    yield();             // Feed the WDT (again)
  }
}

void IRController::send(const char* fileName) {
  char* text = sd.readFile(("/" + std::string(fileName)).c_str());

  // Convert the results into an array suitable for sendRaw().
  // resultToRawArray() allocates the memory we need for the array.
  uint16_t *raw_array;

  // Find out how many elements are in the array.
  uint16_t length;
  length = makeArrayFromText(raw_array, text);

#ifdef EASYDEBUG
  Serial.print("Send Test output : ");
  for (int i = 0; i < length; i++) {
    Serial.print(raw_array[i]);
  }
  Serial.println();
#endif

  // Send it out via the IR LED circuit.
  irsend.sendRaw(raw_array, length, kFrequency);

  // Resume capturing IR messages. It was not restarted until after we sent
  // the message so we didn't capture our own message.
  if(isReading())
    irrecv.resume();

  // Deallocate the memory allocated by resultToRawArray().
  free(raw_array);

#ifdef EASYDEBUG
  // Display a crude timestamp & notification.
  uint32_t now = millis();
  Serial.printf(
    "%06u.%03u: A message that was %d entries long was retransmitted.\n",
    now / 1000, now % 1000, length
  );
#endif

  yield();  // Or delay(milliseconds); This ensures the ESP doesn't WDT reset.
}

void IRController::start() {
  irrecv.enableIRIn();  // Start the receiver
  reading = true;
}

void IRController::stop() {
  irrecv.disableIRIn();  // Stop the receiver
  reading = false;
}

void IRController::makeText(uint16_t *raw_array, uint16_t length, char* text) {
  // Create the text array
  sprintf(text, "raw_array:[");
  for (int i = 0; i < length; i++) {
    sprintf(text + strlen(text), "%d", raw_array[i]);
    if (i != length - 1) {
      sprintf(text + strlen(text), ",");
    }
  }
  sprintf(text + strlen(text), "]\0 ");
}

uint16_t IRController::makeArrayFromText(uint16_t* &rawCode, char *text) {
  // Convert the text array back to raw_array and length
  char* raw_array_start = strstr(text, "raw_array:[") + strlen("raw_array:[");
  char* raw_array_end = strstr(raw_array_start, "]");
  *raw_array_end = '\0';  // Terminate the string at the end of the raw_array

  // Allocate memory for the rawCode
  rawCode = (uint16_t *)malloc(sizeof(uint16_t) * (raw_array_end - raw_array_start + 1));

  // Split the string into elements and convert them to integers
  char* element = strtok(raw_array_start, ",");
  int i = 0;
  for (i; element != NULL; i++) {
    rawCode[i] = atoi(element);
    element = strtok(NULL, ",");
  }
  
  return i;
}
