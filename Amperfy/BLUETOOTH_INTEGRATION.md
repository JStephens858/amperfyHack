# Bluetooth Communication Integration Guide

## Overview

This guide explains how to integrate the Bluetooth communication system into your Amperfy app to enable real-time music playback updates and library querying over Bluetooth Low Energy.

## Architecture

The Bluetooth communication system consists of four main components:

1. **BluetoothManager**: Handles BLE device discovery and connection
2. **BluetoothCommunicationService**: Manages the protocol and message handling
3. **BluetoothPlayerObserver**: Monitors player state and triggers events
4. **BluetoothProtocol**: Defines message formats and constants

## Integration Steps

### 1. Setup in App Initialization

You need to provide the BluetoothManager with references to your player and storage. This should be done after the Bluetooth connection is established.

```swift
// In your app's initialization or settings view
let bluetoothManager = BluetoothManager()

// After connecting to a device
if let player = appDelegate.player,
   let storage = appDelegate.storage {
    bluetoothManager.setupCommunication(player: player, storage: storage)
}
```

### 2. Accessing App Components

The communication service needs access to:
- **PlayerFacade**: To read current playback state
- **LibraryStorage**: To query playlists, artists, albums, and songs

**Find these in your app:**

The exact location depends on your app architecture, but typically:

```swift
// Common patterns to find these:

// From AppDelegate
let appDelegate = UIApplication.shared.delegate as? AppDelegate
let player = appDelegate?.player
let storage = appDelegate?.storage

// From BackendProxy (if using)
let backendProxy = BackendProxy.shared
let player = backendProxy.player
let storage = backendProxy.storage

// From dependency injection container
let player = dependencyContainer.resolve(PlayerFacade.self)
let storage = dependencyContainer.resolve(LibraryStorage.self)
```

### 3. Update BluetoothSettingsView

Add communication setup when connecting to a device:

```swift
struct BluetoothSettingsView: View {
    @StateObject private var bluetoothManager = BluetoothManager()
    @EnvironmentObject private var backendProxy: BackendProxy  // or your DI solution
    
    var body: some View {
        // ... existing UI code ...
        
        .onAppear {
            // Setup communication when view appears
            if let player = backendProxy.player,
               let storage = backendProxy.storage {
                bluetoothManager.setupCommunication(
                    player: player,
                    storage: storage
                )
            }
        }
        .onDisappear {
            // Cleanup when view disappears (optional)
            bluetoothManager.teardownCommunication()
        }
    }
}
```

### 4. Alternative: Singleton Pattern

If you want BluetoothManager to be a singleton:

```swift
@MainActor
class BluetoothManager: NSObject, ObservableObject {
    static let shared = BluetoothManager()
    
    // ... existing code ...
}

// Then in your app initialization:
Task { @MainActor in
    if let player = appDelegate.player,
       let storage = appDelegate.storage {
        BluetoothManager.shared.setupCommunication(
            player: player,
            storage: storage
        )
    }
}
```

## What Gets Sent Automatically

Once configured, the system automatically sends:

### During Playback
- **SONG_STARTED**: When a new song begins playing
- **PLAYBACK_PROGRESS**: Every 250ms with elapsed time
- **SONG_STOPPED**: When playback stops or pauses

### On Device Queries
The connected device can request:
- All playlists
- All artists
- All albums
- All songs
- Songs from a specific playlist/artist/album

Responses are sent automatically.

## Player Integration Details

The `BluetoothPlayerObserver` polls the player every 500ms to detect:
- Song changes (new song starts)
- Play/pause state changes
- Playlist context

### Why Polling?

The current implementation uses polling because it's universal and doesn't require modifying the existing player code. For better performance, you could:

1. **Hook into player notifications** (if your player has them)
2. **Use Combine publishers** (if PlayerFacade is observable)
3. **Add delegate callbacks** to PlayerFacade

Example with notifications:
```swift
// In BluetoothPlayerObserver
NotificationCenter.default.addObserver(
    self,
    selector: #selector(playerDidStartPlaying),
    name: .playerDidStartPlaying,
    object: nil
)
```

## Library Storage Integration

The communication service calls these methods on LibraryStorage:

```swift
// Assuming these methods exist:
storage.main.library.getPlaylists()
storage.main.library.getArtists()
storage.main.library.getAlbums()
storage.main.library.getSongs()
storage.main.library.getPlaylist(id: String)
storage.main.library.getArtist(id: String)
storage.main.library.getAlbum(id: String)
```

**If these methods don't exist**, you'll need to:
1. Add them to your LibraryStorage implementation
2. Or modify `BluetoothCommunicationService` to use your existing API

## Error Handling

The system includes comprehensive error handling:

- Invalid messages → ERROR response sent to device
- Missing data (playlist not found, etc.) → ERROR response
- Connection issues → Logged and reported in UI

Check logs with filter: `category: Bluetooth*`

## Testing

### Test Without a Physical Device

You can test the protocol implementation with:

1. **Nordic nRF Connect app** (iOS/Android)
   - Shows raw characteristics
   - Can send/receive JSON messages manually

2. **Custom test device** using ESP32 or similar
   - Implement the client side of the protocol
   - See BLUETOOTH_PROTOCOL.md for details

### Manual Testing

```swift
// In a debug view or console
let testPayload = SongStartedPayload(
    songId: "test-123",
    title: "Test Song",
    artist: "Test Artist",
    album: "Test Album",
    duration: 180.0,
    playlistName: "Test Playlist",
    playlistId: "playlist-1"
)

let message = BluetoothMessage(type: .songStarted, payload: testPayload)
if let data = message.toData() {
    print("Message size: \(data.count) bytes")
    print("JSON: \(String(data: data, encoding: .utf8) ?? "")")
}
```

## Performance Considerations

### Message Frequency
- Progress updates: 4 messages/second (250ms interval)
- Library queries: On-demand only
- Connection status: As needed

### Data Usage
- Each progress update: ~100-150 bytes
- Song started: ~200-300 bytes
- Library queries: Variable (can be large)

### Battery Impact
- BLE is low power by design
- Progress timer runs only during playback
- Timer stops when playback stops
- Consider increasing interval if battery is a concern

## Troubleshooting

### "Library storage not available"
- Ensure `setupCommunication()` is called after connection
- Check that storage reference is valid

### No playback events
- Verify player reference is set
- Check that songs are actually playing
- Look for logs in Console.app

### Device can't receive messages
- Ensure device subscribed to RX characteristic notifications
- Check message size (must be < 512 bytes)
- Verify JSON encoding is correct

### Large Library Issues
If you have a huge library (10,000+ songs):
- Responses might be too large
- Consider implementing pagination
- Or filter results in queries

## Next Steps

1. **Add the files to your Xcode target**
2. **Integrate with your app's initialization**
3. **Test with a BLE device or simulator**
4. **Customize the protocol if needed**
5. **Consider adding control commands** (play, pause, skip)

## File Checklist

Make sure these files are added to your target:
- ✅ BluetoothManager.swift
- ✅ BluetoothProtocol.swift
- ✅ BluetoothCommunicationService.swift
- ✅ BluetoothPlayerObserver.swift
- ✅ BluetoothSettingsView-Amperfy.swift

## Additional Resources

- [Core Bluetooth Programming Guide](https://developer.apple.com/library/archive/documentation/NetworkingInternetWeb/Conceptual/CoreBluetooth_concepts/)
- [Nordic UART Service Specification](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/bluetooth_services/services/nus.html)
- BLUETOOTH_PROTOCOL.md - Complete protocol specification
