# Bluetooth Connection Persistence Fix

## Problem
When navigating away from the Bluetooth settings screen, the app was disconnecting from the connected device. This happened because the `BluetoothManager` was being destroyed when the settings view was dismissed.

## Solution
Made the following changes to ensure the Bluetooth connection persists:

### 1. Converted BluetoothManager to Singleton Pattern

**File: `BluetoothManager.swift`**

```swift
@MainActor
class BluetoothManager: NSObject, ObservableObject {
  static let shared = BluetoothManager()
  
  // ... rest of the class
  
  private override init() {
    super.init()
    centralManager = CBCentralManager(delegate: self, queue: .main)
  }
}
```

**Changes:**
- Added `static let shared = BluetoothManager()` to create a singleton instance
- Changed `init()` to `private override init()` to prevent external instantiation
- This ensures only one instance exists throughout the app's lifetime

### 2. Updated BluetoothSettingsView

**File: `BluetoothSettingsView.swift`**

```swift
struct BluetoothSettingsView: View {
  @ObservedObject private var bluetoothManager = BluetoothManager.shared
  // ...
}
```

**Changes:**
- Changed from `@StateObject` to `@ObservedObject`
- Changed from `BluetoothManager()` to `BluetoothManager.shared`
- This prevents the view from owning the manager and destroying it on dismiss

### 3. Initialize Bluetooth Communication in SettingsHostVC

**File: `SettingsHostVC.swift`**

```swift
override func viewIsAppearing(_ animated: Bool) {
  super.viewIsAppearing(animated)
  // ... existing code ...
  
  // Setup Bluetooth communication service with player and storage
  Task { @MainActor in
    BluetoothManager.shared.setupCommunication(
      player: appDelegate.player,
      storage: appDelegate.storage.main
    )
  }
  
  // ... rest of the method
}
```

**Changes:**
- Added initialization of the Bluetooth communication service when settings appear
- This ensures the player and storage references are properly set
- Uses `Task { @MainActor in }` to ensure proper actor isolation

## Result

Now when you:
1. Connect to a Bluetooth device in settings
2. Navigate away from the settings screen
3. The connection **persists** because the singleton `BluetoothManager` remains alive

The device will stay connected until you explicitly:
- Tap the "Disconnect" button
- Turn off Bluetooth
- Move out of range
- The peripheral disconnects

## Technical Notes

- The singleton pattern is appropriate here because there should only be one Bluetooth manager per app
- The manager is `@MainActor` isolated to ensure all UI updates happen on the main thread
- The communication service and player observer are also kept alive as properties of the singleton
- No changes were needed to `BluetoothCommunicationService.swift` - it already handles connection lifecycle properly

## Testing

To verify the fix:
1. Open Settings → Bluetooth
2. Scan for devices
3. Connect to your ESP32 device (named "Amperfy-ESP32")
4. Navigate back to main Settings
5. Connection indicator should remain "Connected"
6. Play a song
7. Check ESP32 Serial Monitor - you should see the song data being transmitted
