//
//  BluetoothProtocol.swift
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

import Foundation

// MARK: - Amperfy Bluetooth Protocol (ABP)

/// Protocol version 1.0
/// All messages are JSON-encoded and sent as UTF-8 data

// MARK: - Message Types

enum MessageType: String, Codable {
  // App -> Device events
  case songStarted = "SONG_STARTED"
  case songStopped = "SONG_STOPPED"
  case playbackProgress = "PLAYBACK_PROGRESS"

  // Device -> App queries
  case queryPlaylists = "QUERY_PLAYLISTS"
  case queryArtists = "QUERY_ARTISTS"
  case queryAlbums = "QUERY_ALBUMS"
  case querySongs = "QUERY_SONGS"
  case queryPlaylistSongs = "QUERY_PLAYLIST_SONGS"
  case queryArtistSongs = "QUERY_ARTIST_SONGS"
  case queryAlbumSongs = "QUERY_ALBUM_SONGS"

  // Device -> App commands
  case playSong = "PLAY_SONG"
  case playPause = "PLAY_PAUSE"
  case nextSong = "NEXT_SONG"
  case prevSong = "PREV_SONG"

  // App -> Device responses
  case playlistsResponse = "PLAYLISTS_RESPONSE"
  case artistsResponse = "ARTISTS_RESPONSE"
  case albumsResponse = "ALBUMS_RESPONSE"
  case songsResponse = "SONGS_RESPONSE"

  // Errors
  case error = "ERROR"
}

// MARK: - Base Message

struct BluetoothMessage {
  let type: MessageType
  let timestamp: TimeInterval
  private let payloadData: Data?
  
  init(type: MessageType, payload: Encodable? = nil) {
    self.type = type
    self.timestamp = Date().timeIntervalSince1970
    if let payload = payload {
      self.payloadData = try? JSONEncoder().encode(payload)
    } else {
      self.payloadData = nil
    }
  }
  
  private init(type: MessageType, timestamp: TimeInterval, payloadData: Data?) {
    self.type = type
    self.timestamp = timestamp
    self.payloadData = payloadData
  }
  
  func decode<T: Decodable>(as type: T.Type) -> T? {
    guard let payloadData = payloadData else { return nil }
    return try? JSONDecoder().decode(type, from: payloadData)
  }
  
  /// Converts the message to JSON data with payload as a nested JSON object (not base64)
  func toData() -> Data? {
    var dict: [String: Any] = [
      "type": type.rawValue,
      "timestamp": timestamp
    ]
    
    // If we have a payload, decode it to a JSON object and embed it directly
    if let payloadData = payloadData,
       let payloadObject = try? JSONSerialization.jsonObject(with: payloadData, options: []) {
      dict["payload"] = payloadObject
    }
    
    return try? JSONSerialization.data(withJSONObject: dict, options: [])
  }
  
  /// Parses a message from JSON data with payload as a nested JSON object
  static func from(data: Data) -> BluetoothMessage? {
    guard let dict = try? JSONSerialization.jsonObject(with: data, options: []) as? [String: Any],
          let typeString = dict["type"] as? String,
          let type = MessageType(rawValue: typeString),
          let timestamp = dict["timestamp"] as? TimeInterval else {
      return nil
    }
    
    var payloadData: Data? = nil
    if let payloadObject = dict["payload"] {
      payloadData = try? JSONSerialization.data(withJSONObject: payloadObject, options: [])
    }
    
    return BluetoothMessage(type: type, timestamp: timestamp, payloadData: payloadData)
  }
}

// MARK: - Event Payloads

struct SongStartedPayload: Codable {
  let songId: String
  let title: String
  let artist: String?
  let album: String?
  let duration: Double  // in seconds
  let playlistName: String?
  let playlistId: String?
}

struct SongStoppedPayload: Codable {
  let songId: String
}

struct PlaybackProgressPayload: Codable {
  let songId: String
  let elapsedTime: Double  // in seconds
  let duration: Double
  let isPlaying: Bool
}

// MARK: - Query Payloads

struct QueryPlaylistSongsPayload: Codable {
  let playlistId: String
}

struct QueryArtistSongsPayload: Codable {
  let artistId: String
}

struct QueryAlbumSongsPayload: Codable {
  let albumId: String
}

// MARK: - Command Payloads

struct PlaySongPayload: Codable {
  let songId: String
  let context: String?     // "playlist", "album", "artist", or nil
  let contextId: String?   // ID of the playlist/album/artist
  let songIndex: Int?      // Index of song in context (for queue building)
}

// MARK: - Response Payloads

struct PlaylistInfo: Codable {
  let id: String
  let name: String
  let songCount: Int
}

struct ArtistInfo: Codable {
  let id: String
  let name: String
  let albumCount: Int
  let songCount: Int
}

struct AlbumInfo: Codable {
  let id: String
  let name: String
  let artist: String?
  let songCount: Int
  let year: Int?
}

struct SongInfo: Codable {
  let id: String
  let title: String
  let artist: String?
  let album: String?
  let duration: Double
  let trackNumber: Int?
}

struct PlaylistsResponsePayload: Codable {
  let playlists: [PlaylistInfo]
  let page: Int
  let totalPages: Int
}

struct ArtistsResponsePayload: Codable {
  let artists: [ArtistInfo]
  let page: Int
  let totalPages: Int
}

struct AlbumsResponsePayload: Codable {
  let albums: [AlbumInfo]
  let page: Int
  let totalPages: Int
}

struct SongsResponsePayload: Codable {
  let songs: [SongInfo]
  let context: String?  // "playlist", "artist", "album", or nil for all songs
  let contextId: String?
  let page: Int
  let totalPages: Int
}

struct ErrorPayload: Codable {
  let code: String
  let message: String
}

// MARK: - Protocol Constants

enum BluetoothProtocolConstants {
  static let protocolVersion = "1.0"
  static let maxMessageSize = 512  // 512 bytes max per message
  static let progressUpdateInterval: TimeInterval = 0.25  // 250ms
}
