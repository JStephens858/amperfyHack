//
//  BluetoothPlayerObserver.swift
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
import Combine
import Foundation
import os.log

// MARK: - BluetoothPlayerObserver

/// Observes player state changes and sends events to connected Bluetooth devices
@MainActor
class BluetoothPlayerObserver: ObservableObject {
  
  private let logger = Logger(subsystem: "io.github.amperfy", category: "BluetoothPlayer")
  private var cancellables = Set<AnyCancellable>()
  private var lastPlayingState = false
  private var lastSongId: String?
  
  weak var communicationService: BluetoothCommunicationService?
  weak var player: PlayerFacade?
  
  init() {}
  
  // MARK: - Setup
  
  func startObserving(player: PlayerFacade, communicationService: BluetoothCommunicationService) {
    self.player = player
    self.communicationService = communicationService
    
    // For now, we'll need to poll the player state
    // In a production app, you'd want to hook into actual player notifications
    setupPolling()
    
    logger.info("Started observing player for Bluetooth updates")
  }
  
  func stopObserving() {
    cancellables.removeAll()
    lastPlayingState = false
    lastSongId = nil
    logger.info("Stopped observing player")
  }
  
  // MARK: - Polling (temporary solution)
  
  private func setupPolling() {
    // Poll every 500ms to check for player state changes
    Timer.publish(every: 0.5, on: .main, in: .common)
      .autoconnect()
      .sink { [weak self] _ in
        Task { @MainActor in
          self?.checkPlayerState()
        }
      }
      .store(in: &cancellables)
  }
  
  private func checkPlayerState() {
    guard let player = player,
          let communicationService = communicationService else {
      return
    }
    
    let isCurrentlyPlaying = player.isPlaying
    let currentSong = player.currentlyPlaying
    let currentSongId = currentSong?.id
    
    // Detect song changes
    if currentSongId != lastSongId {
      // Song changed
      if let lastId = lastSongId {
        // Stop the previous song
        communicationService.sendSongStopped(songId: lastId)
      }
      
      if let song = currentSong, isCurrentlyPlaying {
        // Start the new song
        let playlistName = player.contextName.isEmpty ? nil : player.contextName
        communicationService.sendSongStarted(song: song, playlistName: playlistName)
      }
      
      lastSongId = currentSongId
      lastPlayingState = isCurrentlyPlaying
    }
    // Detect play/pause changes on the same song
    else if isCurrentlyPlaying != lastPlayingState {
      if let song = currentSong {
        if isCurrentlyPlaying {
          // Resumed playback
          let playlistName = player.contextName.isEmpty ? nil : player.contextName
          communicationService.sendSongStarted(song: song, playlistName: playlistName)
        } else {
          // Paused playback
          communicationService.sendSongStopped(songId: song.id)
        }
      }
      
      lastPlayingState = isCurrentlyPlaying
    }
  }
}
