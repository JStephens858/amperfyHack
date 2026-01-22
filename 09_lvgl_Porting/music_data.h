/*
 * Music Data Structures and Declarations
 * Car Music Player Display
 */
#pragma once

#include <stdint.h>

// Song structure
struct Song {
    uint16_t id;
    const char* title;
    const char* artist;
    const char* album;
    uint16_t duration_sec;
};

// Album structure
struct Album {
    uint16_t id;
    const char* name;
    const char* artist;
    const Song** songs;
    uint8_t song_count;
};

// Artist structure
struct Artist {
    uint16_t id;
    const char* name;
    const Album** albums;
    uint8_t album_count;
};

// Playlist structure
struct Playlist {
    uint16_t id;
    const char* name;
    const Song** songs;
    uint8_t song_count;
};

// Global data access
extern const Song* const ALL_SONGS[];
extern const uint8_t ALL_SONGS_COUNT;

extern const Album* const ALL_ALBUMS[];
extern const uint8_t ALL_ALBUMS_COUNT;

extern const Artist* const ALL_ARTISTS[];
extern const uint8_t ALL_ARTISTS_COUNT;

extern const Playlist* const ALL_PLAYLISTS[];
extern const uint8_t ALL_PLAYLISTS_COUNT;

// Helper functions
const Song* get_song_by_id(uint16_t id);
const Album* get_album_by_id(uint16_t id);
const Artist* get_artist_by_id(uint16_t id);
const Playlist* get_playlist_by_id(uint16_t id);

// Format duration as "M:SS" string (returns pointer to static buffer)
const char* format_duration(uint16_t seconds);
