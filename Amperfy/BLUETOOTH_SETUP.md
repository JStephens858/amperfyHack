# Bluetooth Setup Instructions

## Required Info.plist Entries

To use Bluetooth functionality in your app, you **must** add the following entries to your `Info.plist` file:

### 1. Bluetooth Usage Description (Required)

```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>Amperfy needs access to Bluetooth to connect to audio devices and peripherals.</string>
```

**Alternative for iOS 12 and earlier (if supporting older versions):**

```xml
<key>NSBluetoothPeripheralUsageDescription</key>
<string>Amperfy needs access to Bluetooth to connect to audio devices and peripherals.</string>
```

### 2. Add CoreBluetooth to your target

Make sure `CoreBluetooth.framework` is linked in your target's build phases.

## Setup Steps

1. **Add Info.plist entry**: Open your `Info.plist` and add the `NSBluetoothAlwaysUsageDescription` key with an appropriate description.

2. **Add both new files to target**: 
   - Select `BluetoothManager.swift` in the Project Navigator
   - Open the File Inspector (⌥⌘1)
   - Check your app target under "Target Membership"
   - Repeat for `BluetoothSettingsView-Amperfy.swift` (or `BluetoothSettingsView.swift`)

3. **Build and run**: Clean build folder (⇧⌘K) and rebuild (⌘B)

## Features Implemented

### BluetoothManager
- ✅ Scan for nearby BLE devices
- ✅ Connect to discovered devices
- ✅ Disconnect from connected devices
- ✅ Track Bluetooth state (enabled/disabled/unauthorized)
- ✅ Error handling and logging
- ✅ RSSI (signal strength) tracking

### BluetoothSettingsView
- ✅ Display Bluetooth status
- ✅ Show connected device info
- ✅ Start/stop scanning with visual feedback
- ✅ List all discovered devices with signal strength
- ✅ Connect/disconnect with tap
- ✅ Error messages display
- ✅ Device count display

## Usage

Users can:
1. Navigate to Settings → Bluetooth
2. Tap "Start Scanning" to discover nearby devices
3. Tap any device in the list to connect
4. Tap the connected device again or use the "Disconnect" button to disconnect
5. Stop scanning when done to save battery

## Technical Details

- Uses **CoreBluetooth** framework for BLE operations
- All operations run on the main actor for thread safety
- Implements proper delegate patterns for CBCentralManager
- Devices are identified by UUID and tracked in an observable array
- RSSI updates are handled for existing devices during scanning
- Logging uses `os.log` for debugging

## Next Steps

You may want to consider:
- Filtering devices by specific service UUIDs
- Discovering and displaying device services/characteristics
- Persisting connected device information
- Auto-reconnecting to previously connected devices
- Adding device-specific features based on services discovered
