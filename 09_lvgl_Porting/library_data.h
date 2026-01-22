/*
 * Dynamic Library Data - BLE-populated music library
 * Stores playlists, artists, albums received from Amperfy app
 */
#pragma once

#include <stdint.h>

// Maximum counts for dynamic data
#define MAX_BLE_PLAYLISTS   50
#define MAX_BLE_ARTISTS     100
#define MAX_BLE_ALBUMS      100
#define MAX_BLE_SONGS       200
#define MAX_NAME_LENGTH     64
#define MAX_ID_LENGTH       48

// Dynamic playlist structure
typedef struct {
    char id[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    uint16_t song_count;
} BLEPlaylist;

// Dynamic artist structure
typedef struct {
    char id[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    uint8_t album_count;
    uint16_t song_count;
} BLEArtist;

// Dynamic album structure
typedef struct {
    char id[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    char artist[MAX_NAME_LENGTH];
    uint8_t song_count;
    uint16_t year;
} BLEAlbum;

// Dynamic song structure
typedef struct {
    char id[MAX_ID_LENGTH];
    char title[MAX_NAME_LENGTH];
    char artist[MAX_NAME_LENGTH];
    char album[MAX_NAME_LENGTH];
    uint16_t duration_sec;
    uint8_t track_number;
} BLESong;

// Initialize library data storage
void library_data_init(void);

// Clear all library data
void library_data_clear(void);

// Check if we have BLE library data
bool library_has_ble_data(void);

// Playlist functions
void library_clear_playlists(void);
bool library_add_playlist(const char* id, const char* name, uint16_t song_count);
uint8_t library_get_playlist_count(void);
const BLEPlaylist* library_get_playlist(uint8_t index);
const BLEPlaylist* library_get_playlist_by_id(const char* id);

// Artist functions
void library_clear_artists(void);
bool library_add_artist(const char* id, const char* name, uint8_t album_count, uint16_t song_count);
uint8_t library_get_artist_count(void);
const BLEArtist* library_get_artist(uint8_t index);
const BLEArtist* library_get_artist_by_id(const char* id);

// Album functions
void library_clear_albums(void);
bool library_add_album(const char* id, const char* name, const char* artist, uint8_t song_count, uint16_t year);
uint8_t library_get_album_count(void);
const BLEAlbum* library_get_album(uint8_t index);
const BLEAlbum* library_get_album_by_id(const char* id);

// Song functions (for playlist/album detail views)
void library_clear_songs(void);
bool library_add_song(const char* id, const char* title, const char* artist, const char* album, uint16_t duration, uint8_t track);
uint8_t library_get_song_count(void);
const BLESong* library_get_song(uint8_t index);

// Context tracking for song lists
void library_set_song_context(const char* context_type, const char* context_id);
const char* library_get_song_context_type(void);
const char* library_get_song_context_id(void);

// Persistent selection tracking (survives reboot)
void library_set_last_playlist_index(uint8_t index);
uint8_t library_get_last_playlist_index(void);

void library_set_last_artist_index(uint8_t index);
uint8_t library_get_last_artist_index(void);

void library_set_last_album_index(uint8_t index);
uint8_t library_get_last_album_index(void);

// Save selections to NVS (call periodically or on selection change)
void library_save_selections(void);

// Load selections from NVS (call on startup)
void library_load_selections(void);
