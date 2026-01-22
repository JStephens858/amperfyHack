/*
 * Bluetooth BLE Module - Amperfy Communication
 * Uses Nordic UART Service (NUS) to communicate with Amperfy app
 */

#include "bluetooth.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Nordic UART Service UUIDs (matching Amperfy protocol)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // Receive from app
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // Transmit to app

// BLE objects
static BLEServer *pServer = nullptr;
static BLECharacteristic *pTxCharacteristic = nullptr;
static BLECharacteristic *pRxCharacteristic = nullptr;

// Connection state
static bool deviceConnected = false;
static bool oldDeviceConnected = false;

// Callbacks
static BLEConnectionCallback connectionCallback = nullptr;
static BLEDataCallback dataCallback = nullptr;

// Server callbacks
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("[BLE] Device connected");
        if (connectionCallback) {
            connectionCallback(true);
        }
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("[BLE] Device disconnected");
        if (connectionCallback) {
            connectionCallback(false);
        }
    }
};

// RX characteristic callbacks (data from app)
class RxCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0) {
            Serial.print("[BLE] Received ");
            Serial.print(rxValue.length());
            Serial.println(" bytes");

            if (dataCallback) {
                dataCallback(rxValue.c_str(), rxValue.length());
            }
        }
    }
};

void bluetooth_init(const char* device_name) {
    Serial.println("[BLE] Initializing Bluetooth...");

    // Create the BLE Device
    BLEDevice::init(device_name);

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create TX Characteristic (ESP32 -> App)
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pTxCharacteristic->addDescriptor(new BLE2902());

    // Create RX Characteristic (App -> ESP32)
    pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE
    );
    pRxCharacteristic->setCallbacks(new RxCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.print("[BLE] Device name: ");
    Serial.println(device_name);
    Serial.println("[BLE] Waiting for connection...");
}

bool bluetooth_is_connected(void) {
    return deviceConnected;
}

void bluetooth_send(const char* data) {
    if (deviceConnected && pTxCharacteristic) {
        pTxCharacteristic->setValue((uint8_t*)data, strlen(data));
        pTxCharacteristic->notify();
    }
}

void bluetooth_send(const uint8_t* data, size_t length) {
    if (deviceConnected && pTxCharacteristic) {
        pTxCharacteristic->setValue((uint8_t*)data, length);
        pTxCharacteristic->notify();
    }
}

void bluetooth_set_connection_callback(BLEConnectionCallback callback) {
    connectionCallback = callback;
}

void bluetooth_set_data_callback(BLEDataCallback callback) {
    dataCallback = callback;
}

void bluetooth_update(void) {
    // Handle disconnecting - restart advertising
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack time to get ready
        pServer->startAdvertising();
        Serial.println("[BLE] Restarting advertising...");
        oldDeviceConnected = deviceConnected;
    }

    // Handle connecting
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}
