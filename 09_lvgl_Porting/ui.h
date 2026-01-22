/*
 * UI Header - Car Music Player Display
 * Touch-based music player interface for 800x480 LCD using LVGL
 */
#pragma once

#include <lvgl.h>
#include "music_data.h"

// Screen types
typedef enum {
    SCREEN_NOW_PLAYING,
    SCREEN_LIBRARY,
    SCREEN_PLAYLISTS,
    SCREEN_ALBUMS,
    SCREEN_ARTISTS,
    SCREEN_PLAYLIST_DETAIL,
    SCREEN_ALBUM_DETAIL,
    SCREEN_ARTIST_ALBUMS
} screen_t;

// Playback state
typedef struct {
    const Song* current_song;
    bool is_playing;
    bool shuffle_enabled;
    bool repeat_enabled;
    uint16_t progress_sec;      // Current playback position
} playback_state_t;

// Initialize the UI system
void ui_init(void);

// Navigation functions
void ui_show_screen(screen_t screen);
void ui_show_now_playing(void);
void ui_show_library(void);
void ui_show_playlists(void);
void ui_show_albums(void);
void ui_show_artists(void);
void ui_show_playlist_detail(const Playlist* playlist);
void ui_show_album_detail(const Album* album);
void ui_show_artist_albums(const Artist* artist);

// BLE navigation functions (for dynamic library data)
void ui_show_ble_playlist_detail(const char* playlist_id, const char* name);
void ui_show_ble_album_detail(const char* album_id, const char* name);
void ui_show_ble_artist_albums(const char* artist_id, const char* name);
void ui_show_ble_songs(void);  // Shows songs after they're loaded from BLE

// Update now playing information (called externally)
void ui_set_current_song(const Song* song);
void ui_set_song_info(const char* title, const char* artist, const char* album, uint16_t duration_sec);
void ui_set_playing(bool playing);
void ui_set_progress(uint16_t progress_sec);
void ui_set_shuffle(bool enabled);
void ui_set_repeat(bool enabled);

// Get current playback state
const playback_state_t* ui_get_playback_state(void);

// Update UI (call periodically if progress needs animation)
void ui_update(void);

// Query callback - called when UI needs to request data from app
typedef void (*UIQueryCallback)(const char* query_type, const char* id);
void ui_set_query_callback(UIQueryCallback callback);

// Play callback - called when UI wants to play a song
// Parameters: song_id, context ("playlist"/"album"/"artist"/NULL), context_id, song_index
typedef void (*UIPlayCallback)(const char* song_id, const char* context, const char* context_id, int song_index);
void ui_set_play_callback(UIPlayCallback callback);

// Command callback - called for playback control commands
// Commands: "PLAY_PAUSE", "NEXT_SONG", "PREV_SONG"
typedef void (*UICommandCallback)(const char* command);
void ui_set_command_callback(UICommandCallback callback);
