/*
 * Dynamic Library Data - BLE-populated music library
 * Stores playlists, artists, albums received from Amperfy app
 */

#include "library_data.h"
#include <string.h>
#include <Arduino.h>
#include <Preferences.h>

// NVS storage
static Preferences g_prefs;
static const char* PREFS_NAMESPACE = "amperfy";

// Last selected indices (persisted)
static uint8_t g_last_playlist_index = 0;
static uint8_t g_last_artist_index = 0;
static uint8_t g_last_album_index = 0;

// Storage arrays
static BLEPlaylist g_playlists[MAX_BLE_PLAYLISTS];
static uint8_t g_playlist_count = 0;

static BLEArtist g_artists[MAX_BLE_ARTISTS];
static uint8_t g_artist_count = 0;

static BLEAlbum g_albums[MAX_BLE_ALBUMS];
static uint8_t g_album_count = 0;

static BLESong g_songs[MAX_BLE_SONGS];
static uint8_t g_song_count = 0;

// Song context tracking
static char g_song_context_type[32] = {0};
static char g_song_context_id[MAX_ID_LENGTH] = {0};

// Flag to track if we have BLE data
static bool g_has_ble_data = false;

// Helper to safely copy strings
static void safe_strcpy(char* dest, const char* src, size_t dest_size) {
    if (src) {
        strncpy(dest, src, dest_size - 1);
        dest[dest_size - 1] = '\0';
    } else {
        dest[0] = '\0';
    }
}

void library_data_init(void) {
    library_data_clear();
}

void library_data_clear(void) {
    library_clear_playlists();
    library_clear_artists();
    library_clear_albums();
    library_clear_songs();
    g_has_ble_data = false;
}

bool library_has_ble_data(void) {
    return g_has_ble_data;
}

// ============================================================================
// Playlists
// ============================================================================

void library_clear_playlists(void) {
    memset(g_playlists, 0, sizeof(g_playlists));
    g_playlist_count = 0;
}

bool library_add_playlist(const char* id, const char* name, uint16_t song_count) {
    if (g_playlist_count >= MAX_BLE_PLAYLISTS) {
        Serial.println("[Library] Max playlists reached");
        return false;
    }

    BLEPlaylist* pl = &g_playlists[g_playlist_count];
    safe_strcpy(pl->id, id, MAX_ID_LENGTH);
    safe_strcpy(pl->name, name, MAX_NAME_LENGTH);
    pl->song_count = song_count;

    g_playlist_count++;
    g_has_ble_data = true;
    return true;
}

uint8_t library_get_playlist_count(void) {
    return g_playlist_count;
}

const BLEPlaylist* library_get_playlist(uint8_t index) {
    if (index >= g_playlist_count) return nullptr;
    return &g_playlists[index];
}

const BLEPlaylist* library_get_playlist_by_id(const char* id) {
    for (uint8_t i = 0; i < g_playlist_count; i++) {
        if (strcmp(g_playlists[i].id, id) == 0) {
            return &g_playlists[i];
        }
    }
    return nullptr;
}

// ============================================================================
// Artists
// ============================================================================

void library_clear_artists(void) {
    memset(g_artists, 0, sizeof(g_artists));
    g_artist_count = 0;
}

bool library_add_artist(const char* id, const char* name, uint8_t album_count, uint16_t song_count) {
    if (g_artist_count >= MAX_BLE_ARTISTS) {
        Serial.println("[Library] Max artists reached");
        return false;
    }

    BLEArtist* artist = &g_artists[g_artist_count];
    safe_strcpy(artist->id, id, MAX_ID_LENGTH);
    safe_strcpy(artist->name, name, MAX_NAME_LENGTH);
    artist->album_count = album_count;
    artist->song_count = song_count;

    g_artist_count++;
    g_has_ble_data = true;
    return true;
}

uint8_t library_get_artist_count(void) {
    return g_artist_count;
}

const BLEArtist* library_get_artist(uint8_t index) {
    if (index >= g_artist_count) return nullptr;
    return &g_artists[index];
}

const BLEArtist* library_get_artist_by_id(const char* id) {
    for (uint8_t i = 0; i < g_artist_count; i++) {
        if (strcmp(g_artists[i].id, id) == 0) {
            return &g_artists[i];
        }
    }
    return nullptr;
}

// ============================================================================
// Albums
// ============================================================================

void library_clear_albums(void) {
    memset(g_albums, 0, sizeof(g_albums));
    g_album_count = 0;
}

bool library_add_album(const char* id, const char* name, const char* artist, uint8_t song_count, uint16_t year) {
    if (g_album_count >= MAX_BLE_ALBUMS) {
        Serial.println("[Library] Max albums reached");
        return false;
    }

    BLEAlbum* album = &g_albums[g_album_count];
    safe_strcpy(album->id, id, MAX_ID_LENGTH);
    safe_strcpy(album->name, name, MAX_NAME_LENGTH);
    safe_strcpy(album->artist, artist, MAX_NAME_LENGTH);
    album->song_count = song_count;
    album->year = year;

    g_album_count++;
    g_has_ble_data = true;
    return true;
}

uint8_t library_get_album_count(void) {
    return g_album_count;
}

const BLEAlbum* library_get_album(uint8_t index) {
    if (index >= g_album_count) return nullptr;
    return &g_albums[index];
}

const BLEAlbum* library_get_album_by_id(const char* id) {
    for (uint8_t i = 0; i < g_album_count; i++) {
        if (strcmp(g_albums[i].id, id) == 0) {
            return &g_albums[i];
        }
    }
    return nullptr;
}

// ============================================================================
// Songs
// ============================================================================

void library_clear_songs(void) {
    memset(g_songs, 0, sizeof(g_songs));
    g_song_count = 0;
    g_song_context_type[0] = '\0';
    g_song_context_id[0] = '\0';
}

bool library_add_song(const char* id, const char* title, const char* artist, const char* album, uint16_t duration, uint8_t track) {
    if (g_song_count >= MAX_BLE_SONGS) {
        Serial.println("[Library] Max songs reached");
        return false;
    }

    BLESong* song = &g_songs[g_song_count];
    safe_strcpy(song->id, id, MAX_ID_LENGTH);
    safe_strcpy(song->title, title, MAX_NAME_LENGTH);
    safe_strcpy(song->artist, artist, MAX_NAME_LENGTH);
    safe_strcpy(song->album, album, MAX_NAME_LENGTH);
    song->duration_sec = duration;
    song->track_number = track;

    g_song_count++;
    return true;
}

uint8_t library_get_song_count(void) {
    return g_song_count;
}

const BLESong* library_get_song(uint8_t index) {
    if (index >= g_song_count) return nullptr;
    return &g_songs[index];
}

void library_set_song_context(const char* context_type, const char* context_id) {
    safe_strcpy(g_song_context_type, context_type, sizeof(g_song_context_type));
    safe_strcpy(g_song_context_id, context_id, sizeof(g_song_context_id));
}

const char* library_get_song_context_type(void) {
    return g_song_context_type;
}

const char* library_get_song_context_id(void) {
    return g_song_context_id;
}

// ============================================================================
// Persistent Selection Tracking
// ============================================================================

void library_set_last_playlist_index(uint8_t index) {
    g_last_playlist_index = index;
}

uint8_t library_get_last_playlist_index(void) {
    return g_last_playlist_index;
}

void library_set_last_artist_index(uint8_t index) {
    g_last_artist_index = index;
}

uint8_t library_get_last_artist_index(void) {
    return g_last_artist_index;
}

void library_set_last_album_index(uint8_t index) {
    g_last_album_index = index;
}

uint8_t library_get_last_album_index(void) {
    return g_last_album_index;
}

void library_save_selections(void) {
    g_prefs.begin(PREFS_NAMESPACE, false);
    g_prefs.putUChar("lastPlaylist", g_last_playlist_index);
    g_prefs.putUChar("lastArtist", g_last_artist_index);
    g_prefs.putUChar("lastAlbum", g_last_album_index);
    g_prefs.end();
    Serial.println("[Library] Saved selections to NVS");
}

void library_load_selections(void) {
    g_prefs.begin(PREFS_NAMESPACE, true);  // read-only
    g_last_playlist_index = g_prefs.getUChar("lastPlaylist", 0);
    g_last_artist_index = g_prefs.getUChar("lastArtist", 0);
    g_last_album_index = g_prefs.getUChar("lastAlbum", 0);
    g_prefs.end();
    Serial.print("[Library] Loaded selections: playlist=");
    Serial.print(g_last_playlist_index);
    Serial.print(", artist=");
    Serial.print(g_last_artist_index);
    Serial.print(", album=");
    Serial.println(g_last_album_index);
}
