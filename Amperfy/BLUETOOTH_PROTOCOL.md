# Amperfy Bluetooth Protocol (ABP) v1.0

## Overview

The Amperfy Bluetooth Protocol (ABP) enables Bluetooth Low Energy devices to receive real-time music playback information from Amperfy and query the music library. All messages are JSON-encoded and transmitted via BLE characteristics.

## BLE Service & Characteristics

The protocol uses the **Nordic UART Service (NUS)** specification:

- **Service UUID**: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- **TX Characteristic** (App → Device): `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- **RX Characteristic** (Device → App): `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`

## Message Format

All messages follow this structure:

```json
{
  "type": "MESSAGE_TYPE",
  "timestamp": 1737302400.0,
  "payload": { ... }
}
```

- `type`: String identifier for the message type
- `timestamp`: Unix timestamp (seconds since epoch)
- `payload`: JSON object containing message-specific data (optional)

**Maximum message size**: 512 bytes

## App → Device Events

### 1. SONG_STARTED

Sent when a song begins playing.

```json
{
  "type": "SONG_STARTED",
  "timestamp": 1737302400.0,
  "payload": {
    "songId": "unique-song-id",
    "title": "Song Title",
    "artist": "Artist Name",
    "album": "Album Name",
    "duration": 245.5,
    "playlistName": "My Playlist",
    "playlistId": "playlist-id"
  }
}
```

**Payload Fields:**
- `songId` (string): Unique identifier for the song
- `title` (string): Song title
- `artist` (string?): Artist name (optional)
- `album` (string?): Album name (optional)
- `duration` (number): Song duration in seconds
- `playlistName` (string?): Name of playlist if playing from one (optional)
- `playlistId` (string?): Playlist ID if applicable (optional)

### 2. SONG_STOPPED

Sent when a song stops playing (paused, stopped, or ended).

```json
{
  "type": "SONG_STOPPED",
  "timestamp": 1737302400.0,
  "payload": {
    "songId": "unique-song-id"
  }
}
```

### 3. PLAYBACK_PROGRESS

Sent every **250ms** while a song is playing.

```json
{
  "type": "PLAYBACK_PROGRESS",
  "timestamp": 1737302400.0,
  "payload": {
    "songId": "unique-song-id",
    "elapsedTime": 120.5,
    "duration": 245.5,
    "isPlaying": true
  }
}
```

**Payload Fields:**
- `songId` (string): ID of the currently playing song
- `elapsedTime` (number): Current playback position in seconds
- `duration` (number): Total song duration in seconds
- `isPlaying` (boolean): Whether playback is active

## Device → App Queries

### 4. QUERY_PLAYLISTS

Request a list of all playlists.

```json
{
  "type": "QUERY_PLAYLISTS",
  "timestamp": 1737302400.0
}
```

**Response**: `PLAYLISTS_RESPONSE`

### 5. QUERY_ARTISTS

Request a list of all artists.

```json
{
  "type": "QUERY_ARTISTS",
  "timestamp": 1737302400.0
}
```

**Response**: `ARTISTS_RESPONSE`

### 6. QUERY_ALBUMS

Request a list of all albums.

```json
{
  "type": "QUERY_ALBUMS",
  "timestamp": 1737302400.0
}
```

**Response**: `ALBUMS_RESPONSE`

### 7. QUERY_SONGS

Request a list of all songs.

```json
{
  "type": "QUERY_SONGS",
  "timestamp": 1737302400.0
}
```

**Response**: `SONGS_RESPONSE`

### 8. QUERY_PLAYLIST_SONGS

Request songs from a specific playlist.

```json
{
  "type": "QUERY_PLAYLIST_SONGS",
  "timestamp": 1737302400.0,
  "payload": {
    "playlistId": "playlist-id"
  }
}
```

**Response**: `SONGS_RESPONSE` with context

### 9. QUERY_ARTIST_SONGS

Request songs from a specific artist.

```json
{
  "type": "QUERY_ARTIST_SONGS",
  "timestamp": 1737302400.0,
  "payload": {
    "artistId": "artist-id"
  }
}
```

**Response**: `SONGS_RESPONSE` with context

### 10. QUERY_ALBUM_SONGS

Request songs from a specific album.

```json
{
  "type": "QUERY_ALBUM_SONGS",
  "timestamp": 1737302400.0,
  "payload": {
    "albumId": "album-id"
  }
}
```

**Response**: `SONGS_RESPONSE` with context

## App → Device Responses

### PLAYLISTS_RESPONSE

```json
{
  "type": "PLAYLISTS_RESPONSE",
  "timestamp": 1737302400.0,
  "payload": {
    "playlists": [
      {
        "id": "playlist-1",
        "name": "My Playlist",
        "songCount": 42
      }
    ]
  }
}
```

### ARTISTS_RESPONSE

```json
{
  "type": "ARTISTS_RESPONSE",
  "timestamp": 1737302400.0,
  "payload": {
    "artists": [
      {
        "id": "artist-1",
        "name": "Artist Name",
        "albumCount": 5,
        "songCount": 50
      }
    ]
  }
}
```

### ALBUMS_RESPONSE

```json
{
  "type": "ALBUMS_RESPONSE",
  "timestamp": 1737302400.0,
  "payload": {
    "albums": [
      {
        "id": "album-1",
        "name": "Album Name",
        "artist": "Artist Name",
        "songCount": 12,
        "year": 2024
      }
    ]
  }
}
```

### SONGS_RESPONSE

```json
{
  "type": "SONGS_RESPONSE",
  "timestamp": 1737302400.0,
  "payload": {
    "songs": [
      {
        "id": "song-1",
        "title": "Song Title",
        "artist": "Artist Name",
        "album": "Album Name",
        "duration": 245.5,
        "trackNumber": 3
      }
    ],
    "context": "playlist",
    "contextId": "playlist-id"
  }
}
```

**Context values:**
- `null`: All songs
- `"playlist"`: Songs from a specific playlist
- `"artist"`: Songs from a specific artist
- `"album"`: Songs from a specific album

### ERROR

```json
{
  "type": "ERROR",
  "timestamp": 1737302400.0,
  "payload": {
    "code": "PLAYLIST_NOT_FOUND",
    "message": "Playlist with ID xyz not found"
  }
}
```

**Error Codes:**
- `MESSAGE_TOO_LARGE`: Message exceeds 512 bytes
- `INVALID_MESSAGE`: Could not decode message
- `NO_STORAGE`: Library storage not available
- `PLAYLIST_NOT_FOUND`: Requested playlist doesn't exist
- `ARTIST_NOT_FOUND`: Requested artist doesn't exist
- `ALBUM_NOT_FOUND`: Requested album doesn't exist

## Communication Flow

### Typical Session

1. **Device connects to Amperfy via BLE**
2. **Device subscribes to RX characteristic** to receive notifications
3. **App discovers characteristics** and sets up communication
4. **App automatically sends playback events**:
   - `SONG_STARTED` when playback begins
   - `PLAYBACK_PROGRESS` every 250ms during playback
   - `SONG_STOPPED` when playback pauses/stops
5. **Device can query library at any time**:
   - Send query messages via TX characteristic
   - Receive responses via RX characteristic notifications
6. **Device disconnects** when done

### Example: Querying a Playlist

```
Device → App: QUERY_PLAYLISTS
App → Device: PLAYLISTS_RESPONSE (list of playlists)
Device → App: QUERY_PLAYLIST_SONGS (specific playlist ID)
App → Device: SONGS_RESPONSE (songs from that playlist)
```

## Implementation Notes

### For Device Developers

1. **Connect to Service**: Scan for devices advertising the NUS service UUID
2. **Discover Characteristics**: Find TX and RX characteristics
3. **Enable Notifications**: Subscribe to RX characteristic
4. **Send Queries**: Write JSON messages to TX characteristic
5. **Receive Events**: Parse JSON from RX characteristic notifications
6. **Handle Large Responses**: Be prepared for large song lists (may need pagination in future versions)

### Message Parsing

All messages should be:
1. UTF-8 decoded
2. JSON parsed
3. Validated for required fields

### Timing

- Progress updates: Every 250ms
- Queries: Responses typically arrive within 100ms
- No strict timeout, but devices should implement their own timeouts

## Future Enhancements

Potential features for future protocol versions:
- Message fragmentation for large responses
- Compression support
- Control commands (play, pause, skip, etc.)
- Authentication/security
- Album artwork transmission
- Lyrics support
- Queue management

## Version History

- **v1.0** (2026-01-19): Initial protocol release
  - Real-time playback events
  - Library querying
  - Basic error handling
