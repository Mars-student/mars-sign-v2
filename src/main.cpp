#include <Arduino.h>
#include <FastLED.h>
#include <NimBLEDevice.h>

#define LED_PIN     3
#define NUM_LEDS    264

// Segment definitions
#define SEG1_LEN    60  // First 60 pixels
#define SEG2_LEN    72  // Next 72 pixels
#define SEG3_LEN    72  // Next 72 pixels
#define SEG4_LEN    60  // Last 60 pixels

// Segment start indices
#define SEG1_START  0
#define SEG2_START  SEG1_LEN
#define SEG3_START  (SEG1_LEN + SEG2_LEN)
#define SEG4_START  (SEG1_LEN + SEG2_LEN + SEG3_LEN)

NimBLEScan* pScan;
static const NimBLEAdvertisedDevice* masterDevice;
NimBLEClient* pClient;
NimBLERemoteCharacteristic* pRemoteCharacteristic;
CRGB leds[NUM_LEDS];
const int SEG_LEN = 66;

int mode = 0;
bool disconnectedOnce = true;
bool connected = false;
bool scanAgain = false;

// BLE UUIDs (must match master)
#define SERVICE_UUID        NimBLEUUID("1234abcd-0000-4000-8000-000000000001")
#define CHARACTERISTIC_UUID NimBLEUUID("1234abcd-0000-4000-8000-000000000002")

// Function prototypes
void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
void connectToMaster();
void runMode();
void modeOff();
void showColorWithSparkle(CRGB color);

/*
class ModeCallback : public NimBLECharacteristicCallbacks {
  void onNotify(NimBLECharacteristic* c) {
    mode = c->getValue()[0];
  }
};
*/

class ClientCallbacks : public NimBLEClientCallbacks {
  void onDisconnect(NimBLEClient* pClient) {
    mode = 0;
    disconnectedOnce = true;
    scanAgain = true;
    connected = false;
    masterDevice = nullptr;
    Serial.println("Disconnected. Restarting scan...");
    pScan->start(5000);
  }
};

class scanCallbacks : public NimBLEScanCallbacks {
  /** Initial discovery, advertisement data only. */
  /*
  void onDiscovered(const NimBLEAdvertisedDevice* advertisedDevice) override {
    //Serial.print("Found Device: ");
    //Serial.println(advertisedDevice->toString().c_str());
    if (advertisedDevice->isAdvertisingService(SERVICE_UUID)) {
      Serial.println("Found Master!");
      pScan->stop();
      masterDevice = advertisedDevice;
    }
  }
  */

  /**
   *  If active scanning the result here will have the scan response data.
   *  If not active scanning then this will be the same as onDiscovered.
   */
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
    //Serial.print("Found Device: ");
    //Serial.println(advertisedDevice->toString().c_str());
    if (advertisedDevice->isAdvertisingService(SERVICE_UUID)) {
      Serial.println("Found Master!");
      pScan->stop();
      masterDevice = advertisedDevice;
    }
  }

  void onScanEnd(const NimBLEScanResults& results, int reason) override {
    printf("Scan ended reason = %d; restarting scan\n", reason);
    if (!masterDevice) {
      scanAgain = true;
    }
  }
} scanCallbacks;

void connectToMaster() {
  Serial.println("Attempting Connection!");

  if (!pClient->connect(masterDevice)) {
    Serial.println("Failed to connect");
    masterDevice = nullptr;
    pScan->start(5000);
    return;
  }

  NimBLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);

  if (!pRemoteService) {
    pClient->disconnect();
    return;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);

  if (!pRemoteCharacteristic) {
    pClient->disconnect();
    return;
  }

  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->subscribe(true, notifyCB);
  }

  connected = true;
  disconnectedOnce = false;
  Serial.println("Connected to Master!");
}

void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  mode = pData[0];
  Serial.println(mode);
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);
  FastLED.clear(true);

  Serial.println("Starting Scan!");
  NimBLEDevice::init("ESP32-SLAVE");

  pClient = NimBLEDevice::createClient();
  pClient->setClientCallbacks(new ClientCallbacks());

  pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(&scanCallbacks, false);
  //pScan->setInterval(100);
  //pScan->setWindow(100);
  pScan->setActiveScan(true);
  pScan->setMaxResults(0);
  pScan->start(5000);

  //connectToMaster();
}

/*
void connectToMaster() {
  NimBLEScanResults results = pScan->getResults();

  for (int i = 0; i < results.getCount(); i++) {
    const NimBLEAdvertisedDevice* device = results.getDevice(i); // Use const pointer
    // Access members with the arrow operator ->
    if (device->haveName()) {
      Serial.println(device->getName().c_str());
      if (strcmp(device->getName().c_str(), "ESP32-MASTER") == 0) {
        NimBLEClient* client = NimBLEDevice::createClient();
        client->connect(device);
        Serial.println("Connection Attempted!");
      

        NimBLERemoteService* svc = client->getService(SERVICE_UUID);
        if (svc) {
          NimBLERemoteCharacteristic* ch = svc->getCharacteristic(CHARACTERISTIC_UUID);
          if (ch) {
            ch->subscribe(true, notifyCB);
          }
        }
      }
    }
  }
}
*/

void loop() {
  //Serial.println("Loop!");
  if (!pClient->isConnected() && !disconnectedOnce) {
    mode = 0;
    disconnectedOnce = true;
    scanAgain = true;
    connected = false;
    masterDevice = nullptr;
    Serial.println("Disconnected. Restarting scan...");
    pScan->start(5000);
  }
  if (scanAgain) {
    scanAgain = false;
    pScan->start(5000);
  }
  if (!connected && masterDevice) {
    connectToMaster();
  }
  runMode();
  FastLED.show();
  delay(40);
}

void runMode() {
  switch (mode) {
    case 0: modeOff(); break;
    //case 1: marquee(CRGB::Blue); break;
    case 1: showColorWithSparkle(CRGB::Blue); break;
    case 2: showColorWithSparkle(CRGB::Red); break;
    //case 3: marquee(CRGB(255, 45, 0)); break;
    case 3: showColorWithSparkle(CRGB(255, 50, 0)); break;
  }
}

void modeOff() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void showColorWithSparkle(CRGB color) {
    // Fill each segment with the base color
    fill_solid(&leds[SEG1_START], SEG1_LEN, color);
    fill_solid(&leds[SEG2_START], SEG2_LEN, color);
    fill_solid(&leds[SEG3_START], SEG3_LEN, color);
    fill_solid(&leds[SEG4_START], SEG4_LEN, color);

    // Pick a random segment and random pixel within that segment to sparkle
    int segment = random(4);
    int sparkleIndex;
    
    switch(segment) {
        case 0: sparkleIndex = SEG1_START + random(SEG1_LEN); break;
        case 1: sparkleIndex = SEG2_START + random(SEG2_LEN); break;
        case 2: sparkleIndex = SEG3_START + random(SEG3_LEN); break;
        case 3: sparkleIndex = SEG4_START + random(SEG4_LEN); break;
    }

    // Make it bright white
    leds[sparkleIndex] = CRGB::White;

    // Show the frame
    FastLED.show();

    // Short sparkle duration
    delay(20);

    // Restore the pixel back to the base color
    leds[sparkleIndex] = color;
}

