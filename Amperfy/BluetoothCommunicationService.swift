//
//  BluetoothCommunicationService.swift
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
import CoreBluetooth
import Foundation
import os.log

// MARK: - BluetoothCommunicationService

/// Handles all Bluetooth communication for the Amperfy protocol
@MainActor
class BluetoothCommunicationService: NSObject, ObservableObject {
  
  private let logger = Logger(subsystem: "io.github.amperfy", category: "BluetoothComm")
  private var progressTimer: Timer?
  private var currentSongId: String?
  
  // References to app components (to be injected)
  weak var player: PlayerFacade?
  weak var storage: LibraryStorage?
  
  // BLE characteristics for communication
  private var txCharacteristic: CBCharacteristic?  // App writes to device
  private var rxCharacteristic: CBCharacteristic?  // App receives from device
  
  // Service and Characteristic UUIDs
  static let serviceUUID = CBUUID(string: "6E400001-B5A3-F393-E0A9-E50E24DCCA9E")  // Nordic UART Service
  static let txCharacteristicUUID = CBUUID(string: "6E400002-B5A3-F393-E0A9-E50E24DCCA9E")
  static let rxCharacteristicUUID = CBUUID(string: "6E400003-B5A3-F393-E0A9-E50E24DCCA9E")
  
  // MARK: - Lifecycle
  
  override init() {
    super.init()
  }
  
  nonisolated deinit {
    // Cannot access @MainActor-isolated properties from deinit
    // Timer will be cleaned up automatically when the object is deallocated
  }
  
  // MARK: - Connection Management
  
  func didConnectToPeripheral(_ peripheral: CBPeripheral) {
    peripheral.delegate = self
    peripheral.discoverServices([Self.serviceUUID])
    logger.info("Discovering services for connected peripheral")
  }
  
  func didDisconnectFromPeripheral() {
    stopProgressTimer()
    txCharacteristic = nil
    rxCharacteristic = nil
    currentSongId = nil
    logger.info("Cleaned up communication service after disconnect")
  }
  
  // MARK: - Sending Events
  
  func sendSongStarted(song: AbstractPlayable, playlistName: String?) {
    let payload = SongStartedPayload(
      songId: song.id,
      title: song.title,
      artist: song.creatorName,
      album: song.asSong?.album?.name,
      duration: Double(song.duration),
      playlistName: playlistName,
      playlistId: nil  // Could be enhanced to include playlist ID
    )
    
    let message = BluetoothMessage(type: .songStarted, payload: payload)
    sendMessage(message)
    
    currentSongId = song.id
    startProgressTimer()
    
    logger.info("Sent song started: \(song.title)")
  }
  
  func sendSongStopped(songId: String) {
    let payload = SongStoppedPayload(songId: songId)
    let message = BluetoothMessage(type: .songStopped, payload: payload)
    sendMessage(message)
    
    stopProgressTimer()
    currentSongId = nil
    
    logger.info("Sent song stopped")
  }
  
  private func sendPlaybackProgress() {
    guard let player = player,
          let currentSong = player.currentlyPlaying,
          let currentSongId = currentSongId else {
      return
    }
    
    let payload = PlaybackProgressPayload(
      songId: currentSongId,
      elapsedTime: player.elapsedTime,
      duration: player.duration,
      isPlaying: player.isPlaying
    )
    
    let message = BluetoothMessage(type: .playbackProgress, payload: payload)
    sendMessage(message)
  }
  
  // MARK: - Timer Management
  
  private func startProgressTimer() {
    stopProgressTimer()
    progressTimer = Timer.scheduledTimer(
      withTimeInterval: BluetoothProtocolConstants.progressUpdateInterval,
      repeats: true
    ) { [weak self] _ in
      Task { @MainActor in
        self?.sendPlaybackProgress()
      }
    }
    logger.debug("Started progress timer")
  }
  
  private func stopProgressTimer() {
    progressTimer?.invalidate()
    progressTimer = nil
    logger.debug("Stopped progress timer")
  }
  
  // MARK: - Message Sending
  
  private func sendMessage(_ message: BluetoothMessage) {
    guard let txCharacteristic = txCharacteristic,
          let peripheral = txCharacteristic.service?.peripheral,
          let data = message.toData() else {
      logger.warning("Cannot send message: missing characteristic or peripheral")
      return
    }
    
    // Check if message is too large
    if data.count > BluetoothProtocolConstants.maxMessageSize {
      logger.error("Message too large: \(data.count) bytes")
      sendError(code: "MESSAGE_TOO_LARGE", message: "Message exceeds max size")
      return
    }
    
    peripheral.writeValue(data, for: txCharacteristic, type: .withResponse)
    logger.debug("Sent message: \(message.type.rawValue) (\(data.count) bytes)")
  }
  
  private func sendError(code: String, message: String) {
    let payload = ErrorPayload(code: code, message: message)
    let errorMessage = BluetoothMessage(type: .error, payload: payload)
    sendMessage(errorMessage)
  }
  
  // MARK: - Message Receiving & Query Handling
  
  private func handleReceivedData(_ data: Data) {
    // Log raw data for debugging
    if let jsonString = String(data: data, encoding: .utf8) {
      logger.info("Received raw data: \(jsonString)")
    }

    guard let message = BluetoothMessage.from(data: data) else {
      logger.error("Failed to decode received message")
      if let jsonString = String(data: data, encoding: .utf8) {
        logger.error("Raw data was: \(jsonString)")
      }
      sendError(code: "INVALID_MESSAGE", message: "Could not decode message")
      return
    }

    logger.info("Received message type: \(message.type.rawValue)")

    Task { @MainActor [weak self] in
      guard let self = self else { return }
      await self.handleQuery(message)
    }
  }
  
  private func handleQuery(_ message: BluetoothMessage) async {
    let storageAvailable = self.storage != nil
    logger.info("Handling query: \(message.type.rawValue), storage available: \(storageAvailable)")

    guard let storage = self.storage else {
      logger.error("Storage is nil! Make sure setupCommunication() was called.")
      sendError(code: "NO_STORAGE", message: "Library storage not available")
      return
    }
    
    switch message.type {
    case .queryPlaylists:
      await handleQueryPlaylists(storage: storage)
      
    case .queryArtists:
      await handleQueryArtists(storage: storage)
      
    case .queryAlbums:
      await handleQueryAlbums(storage: storage)
      
    case .querySongs:
      await handleQuerySongs(storage: storage)
      
    case .queryPlaylistSongs:
      if let payload = message.decode(as: QueryPlaylistSongsPayload.self) {
        await handleQueryPlaylistSongs(storage: storage, playlistId: payload.playlistId)
      }
      
    case .queryArtistSongs:
      if let payload = message.decode(as: QueryArtistSongsPayload.self) {
        await handleQueryArtistSongs(storage: storage, artistId: payload.artistId)
      }
      
    case .queryAlbumSongs:
      if let payload = message.decode(as: QueryAlbumSongsPayload.self) {
        await handleQueryAlbumSongs(storage: storage, albumId: payload.albumId)
      }

    case .playSong:
      if let payload = message.decode(as: PlaySongPayload.self) {
        await handlePlaySong(storage: storage, payload: payload)
      }

    case .playPause:
      handlePlayPause()

    case .nextSong:
      handleNextSong()

    case .prevSong:
      handlePrevSong()

    default:
      logger.warning("Unhandled message type: \(message.type.rawValue)")
    }
  }
  
  // MARK: - Query Handlers

  // Items per page to fit within 512 byte BLE message limit
  // Songs have more fields so need fewer per page
  private let playlistsPerPage = 4
  private let artistsPerPage = 4
  private let albumsPerPage = 3
  private let songsPerPage = 2  // Songs have the most data per item
  private let delayBetweenPages: UInt64 = 50_000_000  // 50ms in nanoseconds

  private func handleQueryPlaylists(storage: LibraryStorage) async {
    let playlists = storage.getAllPlaylists(areSystemPlaylistsIncluded: false)
    let playlistInfos = playlists.map { playlist in
      PlaylistInfo(
        id: playlist.id,
        name: String(playlist.name.prefix(30)),
        songCount: playlist.playables.count
      )
    }

    await sendPaginatedPlaylists(playlistInfos)
    logger.info("Sent all \(playlistInfos.count) playlists")
  }

  private func sendPaginatedPlaylists(_ items: [PlaylistInfo]) async {
    let perPage = playlistsPerPage
    let totalPages = max(1, (items.count + perPage - 1) / perPage)

    for page in 0..<totalPages {
      let start = page * perPage
      let end = min(start + perPage, items.count)
      let pageItems = Array(items[start..<end])

      let payload = PlaylistsResponsePayload(
        playlists: pageItems,
        page: page + 1,
        totalPages: totalPages
      )
      let message = BluetoothMessage(type: .playlistsResponse, payload: payload)
      sendMessage(message)

      logger.debug("Sent playlists page \(page + 1)/\(totalPages)")

      if page < totalPages - 1 {
        try? await Task.sleep(nanoseconds: delayBetweenPages)
      }
    }
  }

  private func handleQueryArtists(storage: LibraryStorage) async {
    let artists = storage.getAllArtists()
    let artistInfos = artists.map { artist in
      ArtistInfo(
        id: artist.id,
        name: String(artist.name.prefix(30)),
        albumCount: artist.albums.count,
        songCount: artist.songs.count
      )
    }

    await sendPaginatedArtists(artistInfos)
    logger.info("Sent all \(artistInfos.count) artists")
  }

  private func sendPaginatedArtists(_ items: [ArtistInfo]) async {
    let perPage = artistsPerPage
    let totalPages = max(1, (items.count + perPage - 1) / perPage)

    for page in 0..<totalPages {
      let start = page * perPage
      let end = min(start + perPage, items.count)
      let pageItems = Array(items[start..<end])

      let payload = ArtistsResponsePayload(
        artists: pageItems,
        page: page + 1,
        totalPages: totalPages
      )
      let message = BluetoothMessage(type: .artistsResponse, payload: payload)
      sendMessage(message)

      logger.debug("Sent artists page \(page + 1)/\(totalPages)")

      if page < totalPages - 1 {
        try? await Task.sleep(nanoseconds: delayBetweenPages)
      }
    }
  }

  private func handleQueryAlbums(storage: LibraryStorage) async {
    let albums = storage.getAllAlbums()
    let albumInfos = albums.map { album in
      AlbumInfo(
        id: album.id,
        name: String(album.name.prefix(25)),
        artist: album.artist.map { String($0.name.prefix(20)) },
        songCount: album.songs.count,
        year: album.year
      )
    }

    await sendPaginatedAlbums(albumInfos)
    logger.info("Sent all \(albumInfos.count) albums")
  }

  private func sendPaginatedAlbums(_ items: [AlbumInfo]) async {
    let perPage = albumsPerPage
    let totalPages = max(1, (items.count + perPage - 1) / perPage)

    for page in 0..<totalPages {
      let start = page * perPage
      let end = min(start + perPage, items.count)
      let pageItems = Array(items[start..<end])

      let payload = AlbumsResponsePayload(
        albums: pageItems,
        page: page + 1,
        totalPages: totalPages
      )
      let message = BluetoothMessage(type: .albumsResponse, payload: payload)
      sendMessage(message)

      logger.debug("Sent albums page \(page + 1)/\(totalPages)")

      if page < totalPages - 1 {
        try? await Task.sleep(nanoseconds: delayBetweenPages)
      }
    }
  }

  private func handleQuerySongs(storage: LibraryStorage) async {
    let songs = storage.getAllSongs()
    let songInfos = songs.map { createSongInfo(from: $0) }

    await sendPaginatedSongs(songInfos, context: nil, contextId: nil)
    logger.info("Sent all \(songInfos.count) songs")
  }

  private func sendPaginatedSongs(_ items: [SongInfo], context: String?, contextId: String?) async {
    let perPage = songsPerPage
    let totalPages = max(1, (items.count + perPage - 1) / perPage)

    for page in 0..<totalPages {
      let start = page * perPage
      let end = min(start + perPage, items.count)
      let pageItems = Array(items[start..<end])

      let payload = SongsResponsePayload(
        songs: pageItems,
        context: context,
        contextId: contextId,
        page: page + 1,
        totalPages: totalPages
      )
      let message = BluetoothMessage(type: .songsResponse, payload: payload)
      sendMessage(message)

      logger.debug("Sent songs page \(page + 1)/\(totalPages)")

      if page < totalPages - 1 {
        try? await Task.sleep(nanoseconds: delayBetweenPages)
      }
    }
  }

  private func handleQueryPlaylistSongs(storage: LibraryStorage, playlistId: String) async {
    let playlists = storage.getAllPlaylists(areSystemPlaylistsIncluded: true)
    guard let playlist = playlists.first(where: { $0.id == playlistId }) else {
      sendError(code: "PLAYLIST_NOT_FOUND", message: "Playlist with ID \(playlistId) not found")
      return
    }

    let songs = playlist.playables.compactMap { $0 as? Song }
    let songInfos = songs.map { createSongInfo(from: $0) }

    await sendPaginatedSongs(songInfos, context: "playlist", contextId: playlistId)
    logger.info("Sent all \(songInfos.count) songs from playlist \(playlist.name)")
  }

  private func handleQueryArtistSongs(storage: LibraryStorage, artistId: String) async {
    let artists = storage.getAllArtists()
    guard let artist = artists.first(where: { $0.id == artistId }) else {
      sendError(code: "ARTIST_NOT_FOUND", message: "Artist with ID \(artistId) not found")
      return
    }

    let songs = artist.songs.compactMap { $0 as? Song }
    let songInfos = songs.map { createSongInfo(from: $0) }

    await sendPaginatedSongs(songInfos, context: "artist", contextId: artistId)
    logger.info("Sent all \(songInfos.count) songs from artist \(artist.name)")
  }

  private func handleQueryAlbumSongs(storage: LibraryStorage, albumId: String) async {
    let albums = storage.getAllAlbums()
    guard let album = albums.first(where: { $0.id == albumId }) else {
      sendError(code: "ALBUM_NOT_FOUND", message: "Album with ID \(albumId) not found")
      return
    }

    let songs = album.songs.compactMap { $0 as? Song }
    let songInfos = songs.map { createSongInfo(from: $0) }

    await sendPaginatedSongs(songInfos, context: "album", contextId: albumId)
    logger.info("Sent all \(songInfos.count) songs from album \(album.name)")
  }

  // MARK: - Playback Command Handlers

  private func handlePlaySong(storage: LibraryStorage, payload: PlaySongPayload) async {
    guard let player = self.player else {
      logger.error("Player is nil! Cannot play song.")
      sendError(code: "NO_PLAYER", message: "Player not available")
      return
    }

    logger.info("Play song request: songId=\(payload.songId), context=\(payload.context ?? "nil"), contextId=\(payload.contextId ?? "nil")")

    var playables: [AbstractPlayable] = []
    var contextName = ""
    var startIndex = 0

    // Build the queue based on context
    if let context = payload.context, let contextId = payload.contextId {
      switch context {
      case "playlist":
        let playlists = storage.getAllPlaylists(areSystemPlaylistsIncluded: true)
        if let playlist = playlists.first(where: { $0.id == contextId }) {
          playables = playlist.playables
          contextName = playlist.name
        }

      case "album":
        let albums = storage.getAllAlbums()
        if let album = albums.first(where: { $0.id == contextId }) {
          playables = album.songs.compactMap { $0 as AbstractPlayable }
          contextName = album.name
        }

      case "artist":
        let artists = storage.getAllArtists()
        if let artist = artists.first(where: { $0.id == contextId }) {
          playables = artist.songs.compactMap { $0 as AbstractPlayable }
          contextName = artist.name
        }

      default:
        logger.warning("Unknown context type: \(context)")
      }
    }

    // If no context or context not found, try to find the song directly
    if playables.isEmpty {
      let songs = storage.getAllSongs()
      if let song = songs.first(where: { $0.id == payload.songId }) {
        playables = [song]
        contextName = song.title
      }
    }

    guard !playables.isEmpty else {
      logger.error("Could not find playables for song \(payload.songId)")
      sendError(code: "SONG_NOT_FOUND", message: "Could not find song or context")
      return
    }

    // Find the index of the selected song by ID (most reliable)
    if let index = playables.firstIndex(where: { $0.id == payload.songId }) {
      startIndex = index
    } else if let songIndex = payload.songIndex {
      // Fall back to provided index if song ID not found
      startIndex = min(songIndex, playables.count - 1)
    }

    logger.info("Playing \(contextName) starting at index \(startIndex) of \(playables.count) songs")

    // Create play context and start playback
    let playContext = PlayContext(name: contextName, index: startIndex, playables: playables)
    player.play(context: playContext)
  }

  private func handlePlayPause() {
    guard let player = self.player else {
      logger.error("Player is nil! Cannot toggle play/pause.")
      return
    }

    player.togglePlayPause()
    logger.info("Toggled play/pause, now playing: \(player.isPlaying)")
  }

  private func handleNextSong() {
    guard let player = self.player else {
      logger.error("Player is nil! Cannot skip to next.")
      return
    }

    player.playNext()
    logger.info("Skipped to next song")
  }

  private func handlePrevSong() {
    guard let player = self.player else {
      logger.error("Player is nil! Cannot skip to previous.")
      return
    }

    player.playPreviousOrReplay()
    logger.info("Skipped to previous song")
  }

  // MARK: - Helper Methods

  private func createSongInfo(from song: Song) -> SongInfo {
    SongInfo(
      id: song.id,
      title: String(song.title.prefix(30)),
      artist: song.artist.map { String($0.name.prefix(20)) },
      album: song.album.map { String($0.name.prefix(20)) },
      duration: Double(song.duration),
      trackNumber: song.track
    )
  }
}

// MARK: - CBPeripheralDelegate

extension BluetoothCommunicationService: CBPeripheralDelegate {
  nonisolated func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
    // Capture peripheral before Task to avoid data races
    nonisolated(unsafe) let unsafePeripheral = peripheral
    
    Task { @MainActor in
      if let error = error {
        logger.error("Error discovering services: \(error.localizedDescription)")
        return
      }
      
      guard let services = unsafePeripheral.services else { return }
      
      for service in services where service.uuid == Self.serviceUUID {
        unsafePeripheral.discoverCharacteristics([Self.txCharacteristicUUID, Self.rxCharacteristicUUID], for: service)
        logger.info("Discovered UART service, discovering characteristics")
      }
    }
  }
  
  nonisolated func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
    // Capture needed values before Task to avoid data races
    nonisolated(unsafe) let unsafePeripheral = peripheral
    nonisolated(unsafe) let unsafeCharacteristics = service.characteristics
    
    Task { @MainActor in
      if let error = error {
        logger.error("Error discovering characteristics: \(error.localizedDescription)")
        return
      }
      
      guard let characteristics = unsafeCharacteristics else { return }
      
      for characteristic in characteristics {
        if characteristic.uuid == Self.txCharacteristicUUID {
          txCharacteristic = characteristic
          logger.info("Found TX characteristic (app sends to device here)")
        } else if characteristic.uuid == Self.rxCharacteristicUUID {
          rxCharacteristic = characteristic
          unsafePeripheral.setNotifyValue(true, for: characteristic)
          logger.info("Found RX characteristic (app receives from device here) - notifications enabled")
          logger.info("Ready to receive queries from device!")
        }
      }
    }
  }
  
  nonisolated func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
    // Capture needed values before Task to avoid data races
    let data = characteristic.value
    
    Task { @MainActor in
      if let error = error {
        logger.error("Error reading characteristic: \(error.localizedDescription)")
        return
      }
      
      guard let data = data else { return }
      handleReceivedData(data)
    }
  }
  
  nonisolated func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
    Task { @MainActor in
      if let error = error {
        logger.error("Error writing characteristic: \(error.localizedDescription)")
      }
    }
  }
}
