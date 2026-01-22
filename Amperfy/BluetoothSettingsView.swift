//
//  BluetoothSettingsView.swift
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

import AmperfyKit
import SwiftUI

// MARK: - BluetoothSettingsView

struct BluetoothSettingsView: View {
  @ObservedObject private var bluetoothManager = BluetoothManager.shared
  
  var body: some View {
    SettingsList {
      // Status Section
      Section {
        HStack {
          Text("Bluetooth Status")
          Spacer()
          if bluetoothManager.isBluetoothEnabled {
            Label("Enabled", systemImage: "checkmark.circle.fill")
              .foregroundColor(.green)
          } else {
            Label("Disabled", systemImage: "xmark.circle.fill")
              .foregroundColor(.red)
          }
        }
        
        if let connectedDevice = bluetoothManager.connectedDevice {
          HStack {
            Text("Connected Device")
            Spacer()
            Text(connectedDevice.name)
              .foregroundColor(.secondary)
          }
          
          Button(role: .destructive) {
            bluetoothManager.disconnect()
          } label: {
            Label("Disconnect", systemImage: "disconnect.circle.fill")
          }
        }
      }
      
      // Error Message Section
      if let errorMessage = bluetoothManager.errorMessage {
        Section {
          HStack {
            Image(systemName: "exclamationmark.triangle.fill")
              .foregroundColor(.orange)
            Text(errorMessage)
              .foregroundColor(.secondary)
          }
        }
      }
      
      // Scanning Section
      Section {
        Button {
          if bluetoothManager.isScanning {
            bluetoothManager.stopScanning()
          } else {
            bluetoothManager.startScanning()
          }
        } label: {
          HStack {
            if bluetoothManager.isScanning {
              ProgressView()
                .padding(.trailing, 8)
              Text("Stop Scanning")
            } else {
              Label("Start Scanning", systemImage: "antenna.radiowaves.left.and.right")
            }
          }
        }
        .disabled(!bluetoothManager.isBluetoothEnabled)
      } header: {
        Text("Scan for Devices")
      } footer: {
        if bluetoothManager.isScanning {
          Text("Scanning for nearby Bluetooth devices...")
        } else {
          Text("Tap to scan for nearby Bluetooth devices")
        }
      }
      
      // Discovered Devices Section
      if !bluetoothManager.discoveredDevices.isEmpty {
        Section {
          ForEach(bluetoothManager.discoveredDevices) { device in
            BluetoothDeviceRow(
              device: device,
              isConnected: bluetoothManager.connectedDevice?.id == device.id
            ) {
              if bluetoothManager.connectedDevice?.id == device.id {
                bluetoothManager.disconnect()
              } else {
                bluetoothManager.connect(to: device)
              }
            }
          }
        } header: {
          Text("Available Devices (\(bluetoothManager.discoveredDevices.count))")
        }
      }
    }
    .navigationTitle("Bluetooth")
    #if !targetEnvironment(macCatalyst)
    .navigationBarTitleDisplayMode(.inline)
    #endif
  }
}

// MARK: - BluetoothDeviceRow

struct BluetoothDeviceRow: View {
  @ObservedObject var device: BluetoothDevice
  let isConnected: Bool
  let onTap: () -> Void
  
  var body: some View {
    Button(action: onTap) {
      HStack {
        VStack(alignment: .leading, spacing: 4) {
          Text(device.name)
            .foregroundColor(.primary)
          
          HStack(spacing: 12) {
            HStack(spacing: 4) {
              Image(systemName: "wave.3.right")
                .font(.caption)
              Text("RSSI: \(device.rssi)")
                .font(.caption)
            }
            
            Text(device.id.uuidString.prefix(8).uppercased())
              .font(.caption)
              .foregroundColor(.secondary)
          }
          .foregroundColor(.secondary)
        }
        
        Spacer()
        
        if isConnected {
          Image(systemName: "checkmark.circle.fill")
            .foregroundColor(.green)
        } else {
          Image(systemName: "circle")
            .foregroundColor(.gray)
        }
      }
    }
    .buttonStyle(.plain)
  }
}

// MARK: - BluetoothSettingsView_Previews

struct BluetoothSettingsView_Previews: PreviewProvider {
  static var previews: some View {
    NavigationView {
      BluetoothSettingsView()
    }
  }
}
