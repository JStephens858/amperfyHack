//
//  BluetoothManager.swift
//  Amperfy
//
//  Created by Maximilian Bauer on 19.01.26.
//  Copyright (c) 2026 Maximilian Bauer. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

import CoreBluetooth
import Foundation
import os.log
import AmperfyKit
import UIKit

// MARK: - BluetoothDevice

/// Represents a discovered Bluetooth device
@MainActor
class BluetoothDevice: Identifiable, ObservableObject {
  let id: UUID
  let peripheral: CBPeripheral
  @Published var name: String
  @Published var rssi: Int
  @Published var isConnected: Bool = false
  
  init(peripheral: CBPeripheral, rssi: Int) {
    self.id = peripheral.identifier
    self.peripheral = peripheral
    self.name = peripheral.name ?? "Unknown Device"
    self.rssi = rssi
  }
  
  func updateRSSI(_ rssi: Int) {
    self.rssi = rssi
  }
}

// MARK: - BluetoothManager

/// Manages Bluetooth Low Energy scanning and connections
@MainActor
class BluetoothManager: NSObject, ObservableObject {
  static let shared = BluetoothManager()
  
  private var centralManager: CBCentralManager!
  
  @Published var isScanning = false
  @Published var isBluetoothEnabled = false
  @Published var discoveredDevices: [BluetoothDevice] = []
  @Published var connectedDevice: BluetoothDevice?
  @Published var errorMessage: String?
  
  private let logger = Logger(subsystem: "io.github.amperfy", category: "Bluetooth")
  
  // Communication service
  let communicationService = BluetoothCommunicationService()
  
  // Player observer
  let playerObserver = BluetoothPlayerObserver()
  
  // Persistent storage keys
  private let lastConnectedDeviceUUIDKey = "LastConnectedBluetoothDeviceUUID"
  private let lastConnectedDeviceNameKey = "LastConnectedBluetoothDeviceName"
  private let shouldAutoReconnectKey = "BluetoothShouldAutoReconnect"
  
  private var hasAttemptedAutoReconnect = false
  
  private override init() {
    super.init()
    centralManager = CBCentralManager(delegate: self, queue: .main)
  }
  
  // MARK: - Persistent Storage
  
  private func saveConnectedDevice(_ device: BluetoothDevice) {
    UserDefaults.standard.set(device.id.uuidString, forKey: lastConnectedDeviceUUIDKey)
    UserDefaults.standard.set(device.name, forKey: lastConnectedDeviceNameKey)
    UserDefaults.standard.set(true, forKey: shouldAutoReconnectKey)
    logger.info("Saved device to persistent storage: \(device.name) (\(device.id.uuidString))")
  }
  
  private func clearConnectedDevice() {
    UserDefaults.standard.removeObject(forKey: lastConnectedDeviceUUIDKey)
    UserDefaults.standard.removeObject(forKey: lastConnectedDeviceNameKey)
    UserDefaults.standard.set(false, forKey: shouldAutoReconnectKey)
    logger.info("Cleared saved device from persistent storage")
  }
  
  private func getLastConnectedDeviceUUID() -> UUID? {
    guard let uuidString = UserDefaults.standard.string(forKey: lastConnectedDeviceUUIDKey),
          let uuid = UUID(uuidString: uuidString) else {
      return nil
    }
    return uuid
  }
  
  private func shouldAutoReconnect() -> Bool {
    return UserDefaults.standard.bool(forKey: shouldAutoReconnectKey)
  }
  
  /// Attempts to reconnect to the last connected device
  func attemptAutoReconnect() {
    guard !hasAttemptedAutoReconnect else {
      logger.debug("Auto-reconnect already attempted")
      return
    }
    
    hasAttemptedAutoReconnect = true
    
    guard shouldAutoReconnect(),
          let lastDeviceUUID = getLastConnectedDeviceUUID() else {
      logger.info("No saved device to reconnect to")
      return
    }
    
    guard centralManager.state == .poweredOn else {
      logger.warning("Cannot auto-reconnect: Bluetooth not powered on")
      return
    }
    
    logger.info("Attempting to reconnect to saved device: \(lastDeviceUUID.uuidString)")
    
    // Retrieve the peripheral from Core Bluetooth
    let peripherals = centralManager.retrievePeripherals(withIdentifiers: [lastDeviceUUID])
    
    guard let peripheral = peripherals.first else {
      logger.warning("Could not retrieve saved peripheral")
      return
    }
    
    // Create a device object and attempt connection
    let device = BluetoothDevice(peripheral: peripheral, rssi: 0)
    discoveredDevices.append(device)
    
    centralManager.connect(peripheral, options: nil)
    logger.info("Initiated reconnection to: \(peripheral.name ?? "Unknown")")
  }
  
  // MARK: - Public Methods
  
  func startScanning() {
    guard centralManager.state == .poweredOn else {
      errorMessage = "Bluetooth is not available"
      logger.warning("Attempted to scan but Bluetooth is not powered on")
      return
    }
    
    discoveredDevices.removeAll()
    centralManager.scanForPeripherals(withServices: nil, options: [CBCentralManagerScanOptionAllowDuplicatesKey: false])
    isScanning = true
    logger.info("Started scanning for BLE devices")
  }
  
  func stopScanning() {
    guard isScanning else { return }
    centralManager.stopScan()
    isScanning = false
    logger.info("Stopped scanning for BLE devices")
  }
  
  func connect(to device: BluetoothDevice) {
    stopScanning()
    centralManager.connect(device.peripheral, options: nil)
    logger.info("Attempting to connect to device: \(device.name)")
  }
  
  func disconnect() {
    guard let device = connectedDevice else { return }
    
    // Clear the saved device so we don't auto-reconnect
    clearConnectedDevice()
    
    centralManager.cancelPeripheralConnection(device.peripheral)
    logger.info("Disconnecting from device: \(device.name)")
  }
  
  // MARK: - Communication Service Integration
  
  func setupCommunication(player: PlayerFacade, storage: LibraryStorage) {
    communicationService.player = player
    communicationService.storage = storage
    playerObserver.startObserving(player: player, communicationService: communicationService)
    logger.info("Communication service configured with player and storage")
  }
  
  func teardownCommunication() {
    playerObserver.stopObserving()
    communicationService.player = nil
    communicationService.storage = nil
    logger.info("Communication service torn down")
  }
  
  // MARK: - Private Methods
  
  private func findDevice(for peripheral: CBPeripheral) -> BluetoothDevice? {
    return discoveredDevices.first { $0.peripheral.identifier == peripheral.identifier }
  }
}

// MARK: - CBCentralManagerDelegate

extension BluetoothManager: CBCentralManagerDelegate {
  nonisolated func centralManagerDidUpdateState(_ central: CBCentralManager) {
    let state = central.state
    Task { @MainActor in
      switch state {
      case .poweredOn:
        isBluetoothEnabled = true
        errorMessage = nil
        logger.info("Bluetooth powered on")
        
        // Attempt auto-reconnect when Bluetooth becomes available
        attemptAutoReconnect()
        
      case .poweredOff:
        isBluetoothEnabled = false
        errorMessage = "Bluetooth is turned off"
        logger.warning("Bluetooth powered off")
      case .unauthorized:
        isBluetoothEnabled = false
        errorMessage = "Bluetooth access not authorized"
        logger.error("Bluetooth unauthorized")
      case .unsupported:
        isBluetoothEnabled = false
        errorMessage = "Bluetooth LE not supported on this device"
        logger.error("Bluetooth unsupported")
      default:
        isBluetoothEnabled = false
        errorMessage = "Bluetooth unavailable"
        logger.warning("Bluetooth state: \(state.rawValue)")
      }
    }
  }
  
  nonisolated func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String: Any], rssi RSSI: NSNumber) {
    let peripheralID = peripheral.identifier
    let peripheralName = peripheral.name
    let rssiValue = RSSI.intValue
    
    Task { @MainActor in
      // Update existing device or add new one
      if let existingDevice = discoveredDevices.first(where: { $0.peripheral.identifier == peripheralID }) {
        existingDevice.updateRSSI(rssiValue)
      } else {
        // Retrieve the peripheral from the central manager on the main actor
        // This is safe because CBCentralManager can be accessed from the main queue
        if let retrievedPeripheral = centralManager.retrievePeripherals(withIdentifiers: [peripheralID]).first {
          let device = BluetoothDevice(peripheral: retrievedPeripheral, rssi: rssiValue)
          discoveredDevices.append(device)
          logger.debug("Discovered device: \(peripheralName ?? "Unknown") with RSSI: \(rssiValue)")
        }
      }
    }
  }
  
  nonisolated func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
    let peripheralID = peripheral.identifier
    Task { @MainActor in
      if let device = discoveredDevices.first(where: { $0.peripheral.identifier == peripheralID }) {
        device.isConnected = true
        connectedDevice = device

        // Save the connected device for auto-reconnect
        saveConnectedDevice(device)

        // Ensure communication service is set up with player and storage
        // This is critical for handling queries from the device
        if communicationService.storage == nil {
          if let appDelegate = UIApplication.shared.delegate as? AppDelegate {
            communicationService.player = appDelegate.player
            communicationService.storage = appDelegate.storage.main.library
            playerObserver.startObserving(player: appDelegate.player, communicationService: communicationService)
            logger.info("Auto-configured communication service on connect")
          } else {
            logger.warning("Could not auto-configure communication service - AppDelegate not available")
          }
        }

        communicationService.didConnectToPeripheral(device.peripheral)
        logger.info("Connected to device: \(device.name)")
      }
    }
  }
  
  nonisolated func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
    let deviceName = peripheral.name ?? "Unknown"
    let errorDescription = error?.localizedDescription
    Task { @MainActor in
      errorMessage = "Failed to connect to \(deviceName)"
      logger.error("Failed to connect to device: \(deviceName), error: \(String(describing: errorDescription))")
    }
  }
  
  nonisolated func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
    let peripheralID = peripheral.identifier
    let hadError = error != nil
    
    Task { @MainActor in
      if let device = discoveredDevices.first(where: { $0.peripheral.identifier == peripheralID }) {
        device.isConnected = false
        if connectedDevice?.id == device.id {
          connectedDevice = nil
        }
        communicationService.didDisconnectFromPeripheral()
        
        if hadError, shouldAutoReconnect() {
          // Unexpected disconnection - try to reconnect
          logger.warning("Unexpected disconnection from device: \(device.name). Attempting to reconnect...")
          
          // Wait a moment before attempting reconnection
          Task { @MainActor in
            try? await Task.sleep(for: .seconds(2))
            if self.centralManager.state == .poweredOn {
              self.centralManager.connect(device.peripheral, options: nil)
            }
          }
        } else {
          logger.info("Disconnected from device: \(device.name)")
        }
      }
    }
  }
}
