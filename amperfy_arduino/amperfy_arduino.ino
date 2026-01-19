/*
 * Amperfy BLE Peripheral for ESP32
 *
 * This Arduino sketch acts as a BLE peripheral that the Amperfy iOS app can
 * connect to via Bluetooth Low Energy (BLE). The iOS app scans for and connects
 * to this device to receive playback information and library queries.
 *
 * Hardware Requirements:
 * - ESP32 development board (ESP32-WROOM, ESP32-DevKitC, etc.)
 * - Optional: 16x2 I2C LCD display (for visual output)
 * - Optional: LED for connection status
 *
 * Software Requirements:
 * - Arduino IDE 1.8.13 or later
 * - ESP32 Board Support (https://github.com/espressif/arduino-esp32)
 * - ArduinoJson library 6.x (Install via Library Manager)
 * - LiquidCrystal_I2C library (if using LCD, install via Library Manager)
 *
 * Features:
 * - BLE peripheral mode - advertises and waits for iOS to connect
 * - Sends playback state updates via notifications
 * - Responds to library query requests from iOS
 * - Auto-reconnect on disconnect
 * - Serial debug output
 *
 * Setup Instructions:
 * 1. Install required libraries via Arduino Library Manager
 * 2. Connect LCD display (if using): SDA to GPIO21, SCL to GPIO22
 * 3. Upload sketch to ESP32
 * 4. Open Serial Monitor (115200 baud)
 * 5. Device will start advertising as "Amperfy_ESP32"
 * 6. Connect from Amperfy iOS app
 *
 * Author: Amperfy Development Team
 * Date: January 18, 2026
 * License: GPL-3.0
 *
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

// Optional: Include LCD library if you want display output
// Uncomment these lines if using LCD:
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 cols, 2 rows

// ============================================================================
// CONFIGURATION
// ============================================================================

// BLE Service and Characteristic UUIDs (from Amperfy protocol)
static BLEUUID serviceUUID("A0000000-1234-5678-9ABC-DEF012345678");
static BLEUUID playerStateUUID("A0000001-1234-5678-9ABC-DEF012345678");
static BLEUUID queryRequestUUID("A0000002-1234-5678-9ABC-DEF012345678");
static BLEUUID queryResponseUUID("A0000003-1234-5678-9ABC-DEF012345678");

// Pin configuration
const int LED_PIN = 2;              // Built-in LED for status

// Display options
const bool USE_LCD = false;         // Set to true if using LCD display
const bool SHOW_SERIAL = true;      // Always show serial output

// Update intervals
const int PLAYER_STATE_UPDATE_INTERVAL = 500;   // Send updates every 500ms
const int DEMO_UPDATE_INTERVAL = 5000;          // Update demo data every 5 seconds

// Device name
const char* DEVICE_NAME = "Amperfy_ESP32";

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// BLE server and characteristics
BLEServer* pServer = nullptr;
BLEService* pService = nullptr;
BLECharacteristic* pPlayerStateChar = nullptr;
BLECharacteristic* pQueryRequestChar = nullptr;
BLECharacteristic* pQueryResponseChar = nullptr;

// Connection state
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Current playback state (demo data - in real use, this would be controlled by external input)
struct PlaybackState {
  String songTitle = "Demo Song";
  String artist = "Demo Artist";
  String album = "Demo Album";
  bool isPlaying = true;
  int duration = 240000;      // milliseconds (4 minutes)
  int elapsed = 0;            // milliseconds
  String playlist = "Demo Playlist";
  unsigned long lastUpdate = 0;
} currentState;

// Timing
unsigned long lastPlayerStateUpdate = 0;
unsigned long lastDemoUpdate = 0;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void setupLCD();
void updateLCD();
void updateSerial();
void sendPlayerStateUpdate();
void sendQueryResponse(const char* type, const char* requestId);
void ledBlink(int times, int delayMs = 100);
void updateDemoPlayback();

// ============================================================================
// BLE CALLBACKS
// ============================================================================

/**
 * Server callbacks - handle connection and disconnection
 */
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    digitalWrite(LED_PIN, HIGH);

    if (SHOW_SERIAL) {
      Serial.println("\n*** iOS CLIENT CONNECTED! ***\n");
    }

    ledBlink(3, 100);
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    digitalWrite(LED_PIN, LOW);

    if (SHOW_SERIAL) {
      Serial.println("\n*** iOS CLIENT DISCONNECTED ***\n");
    }
  }
};

/**
 * Callback for Query Request characteristic
 * iOS writes queries here, we respond via Query Response characteristic
 */
class QueryRequestCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    //String value = String(stdValue.c_str());

    if (value.length() > 0) {
      if (SHOW_SERIAL) {
        Serial.println("\n=== Query Request Received ===");
        Serial.print("Data: ");
        Serial.println(value);
      }

      // Parse the query request
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, value.c_str());

      if (error) {
        if (SHOW_SERIAL) {
          Serial.print("JSON parse failed: ");
          Serial.println(error.c_str());
        }
        return;
      }

      const char* type = doc["type"];
      const char* requestId = doc["requestId"];

      if (SHOW_SERIAL) {
        Serial.print("Query Type: ");
        Serial.println(type);
        Serial.print("Request ID: ");
        Serial.println(requestId);
      }

      // Send appropriate response
      sendQueryResponse(type, requestId);

      if (SHOW_SERIAL) {
        Serial.println("===================\n");
      }
    }
  }
};

// ============================================================================
// BLE SERVER SETUP FUNCTIONS
// ============================================================================

/**
 * Initialize BLE server and start advertising
 */
void setupBLEServer() {
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  pService = pServer->createService(serviceUUID);

  // Create Player State characteristic (NOTIFY)
  pPlayerStateChar = pService->createCharacteristic(
    playerStateUUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pPlayerStateChar->addDescriptor(new BLE2902());

  // Create Query Request characteristic (WRITE)
  pQueryRequestChar = pService->createCharacteristic(
    queryRequestUUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pQueryRequestChar->setCallbacks(new QueryRequestCallbacks());

  // Create Query Response characteristic (NOTIFY)
  pQueryResponseChar = pService->createCharacteristic(
    queryResponseUUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pQueryResponseChar->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  if (SHOW_SERIAL) {
    Serial.println("BLE service created and started");
  }
}

/**
 * Start advertising the BLE service
 */
void startAdvertising() {
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // iPhone connection interval
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  if (SHOW_SERIAL) {
    Serial.println("BLE advertising started");
    Serial.print("Device name: ");
    Serial.println(DEVICE_NAME);
    Serial.println("\nWaiting for iOS client to connect...\n");
  }
}

// ============================================================================
// NOTIFICATION FUNCTIONS
// ============================================================================

/**
 * Send player state update to iOS client via notification
 */
void sendPlayerStateUpdate() {
  if (!deviceConnected) {
    return;
  }

  // Build JSON with current playback state
  StaticJsonDocument<1024> doc;
  doc["isPlaying"] = currentState.isPlaying;
  doc["playlist"] = currentState.playlist;

  JsonObject song = doc.createNestedObject("currentSong");
  song["title"] = currentState.songTitle;
  song["artist"] = currentState.artist;
  song["album"] = currentState.album;
  song["duration"] = currentState.duration;
  song["elapsed"] = currentState.elapsed;

  // Serialize to string
  String json;
  serializeJson(doc, json);

  // Send notification
  pPlayerStateChar->setValue(json.c_str());
  pPlayerStateChar->notify();

  lastPlayerStateUpdate = millis();
}

/**
 * Send query response to iOS client
 * This is a demo implementation - in a real system, this would query actual library data
 */
void sendQueryResponse(const char* type, const char* requestId) {
  if (!deviceConnected) {
    return;
  }

  DynamicJsonDocument doc(4096);
  doc["requestId"] = requestId;

  if (strcmp(type, "queryPlaylists") == 0) {
    doc["type"] = "playlistsResponse";
    JsonArray playlists = doc.createNestedArray("playlists");

    // Demo data - in reality, this would come from actual library
    JsonObject pl1 = playlists.createNestedObject();
    pl1["id"] = "playlist_1";
    pl1["name"] = "Favorites";
    pl1["songCount"] = 42;

    JsonObject pl2 = playlists.createNestedObject();
    pl2["id"] = "playlist_2";
    pl2["name"] = "Rock";
    pl2["songCount"] = 128;

  } else if (strcmp(type, "queryArtists") == 0) {
    doc["type"] = "artistsResponse";
    JsonArray artists = doc.createNestedArray("artists");

    JsonObject ar1 = artists.createNestedObject();
    ar1["id"] = "artist_1";
    ar1["name"] = "Demo Artist 1";
    ar1["albumCount"] = 5;

    JsonObject ar2 = artists.createNestedObject();
    ar2["id"] = "artist_2";
    ar2["name"] = "Demo Artist 2";
    ar2["albumCount"] = 3;

  } else {
    // Unknown query type - return error
    doc["type"] = "error";
    doc["errorCode"] = "UNKNOWN_QUERY";
    doc["message"] = "Query type not supported";
  }

  // Serialize and send
  String json;
  serializeJson(doc, json);

  pQueryResponseChar->setValue(json.c_str());
  pQueryResponseChar->notify();

  if (SHOW_SERIAL) {
    Serial.println("Query response sent");
  }
}

// ============================================================================
// DISPLAY FUNCTIONS
// ============================================================================

/**
 * Setup LCD display (if using)
 */
void setupLCD() {
  // Uncomment if using LCD:
  /*
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Amperfy Client");
  lcd.setCursor(0, 1);
  lcd.print("Searching...");
  */
}

/**
 * Update LCD display with current state
 */
void updateLCD() {
  // Uncomment and customize if using LCD:
  /*
  lcd.clear();
  
  if (currentState.songTitle.length() > 0) {
    // Line 1: Song title (scrolling if too long)
    String line1 = currentState.songTitle;
    if (line1.length() > 16) {
      line1 = line1.substring(0, 16);
    }
    lcd.setCursor(0, 0);
    lcd.print(line1);
    
    // Line 2: Artist + play status
    String line2 = currentState.artist;
    if (line2.length() > 13) {
      line2 = line2.substring(0, 13);
    }
    lcd.setCursor(0, 1);
    lcd.print(line2);
    lcd.setCursor(14, 1);
    lcd.print(currentState.isPlaying ? ">" : "||");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("No song playing");
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }
  */
}

/**
 * Update Serial Monitor with current state
 */
void updateSerial() {
  Serial.print("Playing: ");
  Serial.println(currentState.isPlaying ? "YES" : "NO");
  
  if (currentState.songTitle.length() > 0) {
    Serial.print("Song: ");
    Serial.println(currentState.songTitle);
    Serial.print("Artist: ");
    Serial.println(currentState.artist);
    Serial.print("Album: ");
    Serial.println(currentState.album);
    
    if (currentState.duration > 0) {
      int elapsedSec = currentState.elapsed / 1000;
      int durationSec = currentState.duration / 1000;
      int elapsedMin = elapsedSec / 60;
      int elapsedRemSec = elapsedSec % 60;
      int durationMin = durationSec / 60;
      int durationRemSec = durationSec % 60;
      
      Serial.print("Progress: ");
      Serial.printf("%d:%02d / %d:%02d", elapsedMin, elapsedRemSec, durationMin, durationRemSec);
      Serial.print(" (");
      Serial.print((currentState.elapsed * 100) / currentState.duration);
      Serial.println("%)");
    }
  }
  
  if (currentState.playlist.length() > 0) {
    Serial.print("Playlist: ");
    Serial.println(currentState.playlist);
  }
  
  Serial.println("===========================");
}

/**
 * Blink LED pattern
 */
void ledBlink(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    delay(delayMs);
  }
}

/**
 * Update demo playback data (simulates song progress)
 * In a real implementation, this would be controlled by actual hardware inputs
 */
void updateDemoPlayback() {
  if (currentState.isPlaying) {
    // Increment elapsed time
    unsigned long now = millis();
    unsigned long deltaTime = now - lastDemoUpdate;
    currentState.elapsed += deltaTime;

    // Loop back to start if we exceed duration
    if (currentState.elapsed >= currentState.duration) {
      currentState.elapsed = 0;
    }

    lastDemoUpdate = now;
  }
}

// ============================================================================
// ARDUINO SETUP AND LOOP
// ============================================================================

void setup() {
  // Initialize serial
  Serial.begin(115200);
  Serial.println();
  Serial.println("=================================");
  Serial.println("  Amperfy BLE Peripheral - ESP32");
  Serial.println("=================================");
  Serial.println();

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  ledBlink(2, 500); // Startup blink

  // Initialize LCD (if using)
  if (USE_LCD) {
    setupLCD();
  }

  // Initialize BLE
  Serial.println("Initializing BLE...");
  BLEDevice::init(DEVICE_NAME);

  // Set up BLE server
  setupBLEServer();

  // Start advertising
  startAdvertising();

  // Initialize timing
  lastPlayerStateUpdate = millis();
  lastDemoUpdate = millis();

  Serial.println("Setup complete!\n");
}

void loop() {
  // Handle reconnection after disconnect
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Give iOS time to process disconnect
    startAdvertising();
    Serial.println("Restarting advertising after disconnect");
    oldDeviceConnected = deviceConnected;
  }

  // Handle new connection
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("New connection established");
  }

  // If connected, send periodic updates
  if (deviceConnected) {
    unsigned long now = millis();

    // Update demo playback state
    updateDemoPlayback();

    // Send player state updates every 500ms
    if (now - lastPlayerStateUpdate >= PLAYER_STATE_UPDATE_INTERVAL) {
      sendPlayerStateUpdate();

      if (SHOW_SERIAL) {
        Serial.print(".");  // Show activity indicator
      }
    }
  }

  delay(100); // Small delay to prevent tight loop
}
