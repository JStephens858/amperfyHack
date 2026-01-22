/*
 * ESP32 Bluetooth Test Script for Amperfy
 * 
 * This script sets up a BLE server using the Nordic UART Service (NUS) UUIDs
 * that match the Amperfy Bluetooth protocol. It waits for connections from
 * the Amperfy app and prints out any data received.
 * 
 * Hardware: ESP32 (any variant with BLE support)
 * 
 * Installation:
 * 1. Install the ESP32 board support in Arduino IDE
 * 2. Upload this sketch to your ESP32
 * 3. Open Serial Monitor at 115200 baud
 * 4. Connect from the Amperfy app
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Nordic UART Service UUIDs (matching Amperfy protocol)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // Receive from app
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // Transmit to app

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
BLECharacteristic *pRxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("\n=== Device Connected ===");
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("\n=== Device Disconnected ===");
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      Serial.println("\n--- Received Data ---");
      Serial.print("Length: ");
      Serial.print(rxValue.length());
      Serial.println(" bytes");
      
      /*
      Serial.print("Hex: ");
      for (int i = 0; i < rxValue.length(); i++) {
        if (rxValue[i] < 16) Serial.print("0");
        Serial.print((uint8_t)rxValue[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      */
      
      Serial.print("ASCII: ");
      for (int i = 0; i < rxValue.length(); i++) {
        char c = rxValue[i];
        if (c >= 32 && c <= 126) {
          Serial.print(c);
        } else {
          Serial.print(".");
        }
      }
      Serial.println();
      
      // Try to parse as JSON if it looks like JSON
      if (rxValue[0] == '{') {
        Serial.print("JSON:  ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();
      }
      
      Serial.println("--------------------");
    }
  }
};

void setup() {
  Serial.begin(115200);
  
  Serial.println("\n\n=================================");
  Serial.println("  Amperfy BLE Test - ESP32");
  Serial.println("=================================\n");

  // Create the BLE Device
  BLEDevice::init("Amperfy-ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics
  // TX Characteristic (ESP32 -> App)
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // RX Characteristic (App -> ESP32)
  pRxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_RX,
                        BLECharacteristic::PROPERTY_WRITE
                      );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE Server initialized");
  Serial.println("Device name: Amperfy-ESP32");
  Serial.println("Waiting for connection...\n");
  Serial.println("Service UUID: " SERVICE_UUID);
  Serial.println("RX UUID:      " CHARACTERISTIC_UUID_RX);
  Serial.println("TX UUID:      " CHARACTERISTIC_UUID_TX);
  Serial.println("\n=================================\n");
}

void loop() {
  // Handle disconnecting and reconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack time to get ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Restarting advertising...");
    oldDeviceConnected = deviceConnected;
  }
  
  // Handle connecting
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
  
  delay(10);
}
