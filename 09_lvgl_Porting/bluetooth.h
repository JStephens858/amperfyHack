/*
 * Bluetooth BLE Module - Amperfy Communication
 * Uses Nordic UART Service (NUS) to communicate with Amperfy app
 */
#pragma once

#include <Arduino.h>

// Callback function types for received data
typedef void (*BLEConnectionCallback)(bool connected);
typedef void (*BLEDataCallback)(const char* data, size_t length);

// Initialize the BLE server
void bluetooth_init(const char* device_name = "Amperfy-ESP32");

// Check if a device is connected
bool bluetooth_is_connected(void);

// Send data to the connected device
void bluetooth_send(const char* data);
void bluetooth_send(const uint8_t* data, size_t length);

// Set callbacks
void bluetooth_set_connection_callback(BLEConnectionCallback callback);
void bluetooth_set_data_callback(BLEDataCallback callback);

// Call periodically to handle BLE events (reconnection, etc.)
void bluetooth_update(void);
