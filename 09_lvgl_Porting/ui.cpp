/*
 * UI Implementation - Car Music Player Display
 * Touch-based music player interface for 800x480 LCD using LVGL
 */
#include "ui.h"
#include "library_data.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// CONSTANTS - Layout and Colors
// ============================================================================

// Screen dimensions
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   480

// Layout
#define HEADER_HEIGHT   120
#define FOOTER_HEIGHT   80
#define CONTENT_HEIGHT  (SCREEN_HEIGHT - HEADER_HEIGHT - FOOTER_HEIGHT)
#define CONTENT_Y       HEADER_HEIGHT

// List settings
#define ITEMS_PER_PAGE  4
#define LIST_ITEM_HEIGHT 80
#define LIST_ITEM_SPACING 10
#define NAV_BUTTON_WIDTH 100

// Colors (dark theme)
#define COLOR_BG            lv_color_hex(0x1a1a1a)
#define COLOR_HEADER_BG     lv_color_hex(0x252525)
#define COLOR_FOOTER_BG     lv_color_hex(0x252525)
#define COLOR_PRIMARY       lv_color_hex(0xffffff)
#define COLOR_SECONDARY     lv_color_hex(0xaaaaaa)
#define COLOR_ACCENT        lv_color_hex(0x2196F3)
#define COLOR_BUTTON_BG     lv_color_hex(0x333333)
#define COLOR_BUTTON_PRESS  lv_color_hex(0x444444)
#define COLOR_ALBUM_ART     lv_color_hex(0x3d3d3d)
#define COLOR_PROGRESS_BG   lv_color_hex(0x444444)
#define COLOR_PROGRESS_FG   lv_color_hex(0x2196F3)

// ============================================================================
// GLOBAL STATE
// ============================================================================

static playback_state_t g_playback = {
    .current_song = nullptr,
    .is_playing = false,
    .shuffle_enabled = false,
    .repeat_enabled = false,
    .progress_sec = 0
};

// Navigation state
static screen_t g_current_screen = SCREEN_NOW_PLAYING;
static uint8_t g_list_page = 0;
static const Playlist* g_selected_playlist = nullptr;
static const Album* g_selected_album = nullptr;
static const Artist* g_selected_artist = nullptr;

// BLE navigation state (stores IDs for BLE data)
static char g_selected_ble_playlist_id[48] = {0};
static char g_selected_ble_album_id[48] = {0};
static char g_selected_ble_artist_id[48] = {0};

// Screen objects
static lv_obj_t* g_screen = nullptr;

// Now Playing screen widgets
static lv_obj_t* g_np_song_title = nullptr;
static lv_obj_t* g_np_artist = nullptr;
static lv_obj_t* g_np_album = nullptr;
static lv_obj_t* g_np_progress_bar = nullptr;
static lv_obj_t* g_np_time_current = nullptr;
static lv_obj_t* g_np_time_total = nullptr;
static lv_obj_t* g_np_btn_play = nullptr;
static lv_obj_t* g_np_btn_shuffle = nullptr;

// Dynamic song info (for BLE data)
static char g_ble_song_title[128] = {0};
static char g_ble_song_artist[128] = {0};
static char g_ble_song_album[128] = {0};
static uint16_t g_ble_song_duration = 0;
static bool g_using_ble_song = false;

// BLE detail screen state
static char g_ble_detail_name[64] = {0};
static char g_ble_detail_id[48] = {0};
static char g_ble_detail_type[16] = {0};  // "playlist", "album", "artist"

// Query callback - set by main code to handle query requests
typedef void (*QueryCallback)(const char* query_type, const char* id);
static QueryCallback g_query_callback = nullptr;

// Play callback - set by main code to handle play requests
typedef void (*PlayCallback)(const char* song_id, const char* context, const char* context_id, int song_index);
static PlayCallback g_play_callback = nullptr;

// Command callback - set by main code to handle playback control commands
typedef void (*CommandCallback)(const char* command);
static CommandCallback g_command_callback = nullptr;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void create_now_playing_screen(void);
static void create_library_screen(void);
static void create_playlists_screen(void);
static void create_albums_screen(void);
static void create_artists_screen(void);
static void create_playlist_detail_screen(const Playlist* playlist);
static void create_album_detail_screen(const Album* album);
static void create_artist_albums_screen(const Artist* artist);
static void create_ble_songs_screen(void);

static void update_now_playing_display(void);
static void on_library_btn_click(lv_event_t* e);
static void on_back_btn_click(lv_event_t* e);
static void on_now_playing_btn_click(lv_event_t* e);

// ============================================================================
// UI HELPERS
// ============================================================================

static lv_obj_t* create_header(const char* title, bool show_back, bool show_now_playing) {
    lv_obj_t* header = lv_obj_create(g_screen);
    lv_obj_set_size(header, SCREEN_WIDTH, HEADER_HEIGHT);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, COLOR_HEADER_BG, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_all(header, 20, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Back button
    if (show_back) {
        lv_obj_t* btn_back = lv_btn_create(header);
        lv_obj_set_size(btn_back, 160, 80);
        lv_obj_align(btn_back, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_bg_color(btn_back, COLOR_BUTTON_BG, 0);
        lv_obj_set_style_bg_color(btn_back, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
        lv_obj_add_event_cb(btn_back, on_back_btn_click, LV_EVENT_CLICKED, nullptr);

        lv_obj_t* lbl = lv_label_create(btn_back);
        lv_label_set_text(lbl, LV_SYMBOL_LEFT " Back");
        lv_obj_set_style_text_color(lbl, COLOR_PRIMARY, 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, 0);
        lv_obj_center(lbl);
    }

    // Title
    lv_obj_t* lbl_title = lv_label_create(header);
    lv_label_set_text(lbl_title, title);
    lv_obj_set_style_text_color(lbl_title, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_30, 0);
    lv_obj_align(lbl_title, LV_ALIGN_CENTER, 0, 0);

    // Now Playing button
    if (show_now_playing) {
        lv_obj_t* btn_np = lv_btn_create(header);
        lv_obj_set_size(btn_np, 240, 80);
        lv_obj_align(btn_np, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_set_style_bg_color(btn_np, COLOR_ACCENT, 0);
        lv_obj_set_style_bg_color(btn_np, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
        lv_obj_add_event_cb(btn_np, on_now_playing_btn_click, LV_EVENT_CLICKED, nullptr);

        lv_obj_t* lbl = lv_label_create(btn_np);
        lv_label_set_text(lbl, LV_SYMBOL_AUDIO " Playing");
        lv_obj_set_style_text_color(lbl, COLOR_PRIMARY, 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, 0);
        lv_obj_center(lbl);
    }

    return header;
}

// Creates side navigation buttons on left side (prev on top, next on bottom)
static void create_side_navigation(uint8_t current_page, uint8_t total_pages,
                                   void (*on_prev)(lv_event_t*),
                                   void (*on_next)(lv_event_t*)) {
    int content_height = SCREEN_HEIGHT - HEADER_HEIGHT;
    int btn_height = (content_height - 30) / 2;  // Two buttons with spacing

    // Prev button (top)
    lv_obj_t* btn_prev = lv_btn_create(g_screen);
    lv_obj_set_size(btn_prev, NAV_BUTTON_WIDTH, btn_height);
    lv_obj_set_pos(btn_prev, 10, HEADER_HEIGHT + 10);
    lv_obj_set_style_bg_color(btn_prev, COLOR_BUTTON_BG, 0);
    lv_obj_set_style_bg_color(btn_prev, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_prev, 10, 0);
    if (current_page == 0) {
        lv_obj_add_state(btn_prev, LV_STATE_DISABLED);
        lv_obj_set_style_bg_opa(btn_prev, LV_OPA_30, LV_STATE_DISABLED);
    }
    lv_obj_add_event_cb(btn_prev, on_prev, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* lbl_prev = lv_label_create(btn_prev);
    lv_label_set_text(lbl_prev, LV_SYMBOL_UP);
    lv_obj_set_style_text_color(lbl_prev, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_prev, &lv_font_montserrat_30, 0);
    lv_obj_center(lbl_prev);

    // Next button (bottom)
    lv_obj_t* btn_next = lv_btn_create(g_screen);
    lv_obj_set_size(btn_next, NAV_BUTTON_WIDTH, btn_height);
    lv_obj_set_pos(btn_next, 10, HEADER_HEIGHT + btn_height + 20);
    lv_obj_set_style_bg_color(btn_next, COLOR_BUTTON_BG, 0);
    lv_obj_set_style_bg_color(btn_next, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_next, 10, 0);
    if (current_page >= total_pages - 1) {
        lv_obj_add_state(btn_next, LV_STATE_DISABLED);
        lv_obj_set_style_bg_opa(btn_next, LV_OPA_30, LV_STATE_DISABLED);
    }
    lv_obj_add_event_cb(btn_next, on_next, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* lbl_next = lv_label_create(btn_next);
    lv_label_set_text(lbl_next, LV_SYMBOL_DOWN);
    lv_obj_set_style_text_color(lbl_next, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_next, &lv_font_montserrat_30, 0);
    lv_obj_center(lbl_next);
}

// Create content area for list screens (offset for side nav)
static lv_obj_t* create_list_content_area(void) {
    lv_obj_t* content = lv_obj_create(g_screen);
    lv_obj_set_size(content, SCREEN_WIDTH - NAV_BUTTON_WIDTH - 30, SCREEN_HEIGHT - HEADER_HEIGHT);
    lv_obj_set_pos(content, NAV_BUTTON_WIDTH + 20, HEADER_HEIGHT);
    lv_obj_set_style_bg_color(content, COLOR_BG, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_radius(content, 0, 0);
    lv_obj_set_style_pad_all(content, 10, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    return content;
}

static lv_obj_t* create_content_area(void) {
    lv_obj_t* content = lv_obj_create(g_screen);
    lv_obj_set_size(content, SCREEN_WIDTH, CONTENT_HEIGHT);
    lv_obj_set_pos(content, 0, CONTENT_Y);
    lv_obj_set_style_bg_color(content, COLOR_BG, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_radius(content, 0, 0);
    lv_obj_set_style_pad_all(content, 20, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    return content;
}

static lv_obj_t* create_list_item(lv_obj_t* parent, const char* primary_text,
                                  const char* secondary_text, uint8_t index,
                                  void (*on_click)(lv_event_t*)) {
    lv_obj_t* item = lv_btn_create(parent);
    lv_obj_set_size(item, lv_pct(100), LIST_ITEM_HEIGHT);
    lv_obj_set_style_bg_color(item, COLOR_BUTTON_BG, 0);
    lv_obj_set_style_bg_color(item, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_set_style_radius(item, 8, 0);
    lv_obj_set_style_pad_hor(item, 20, 0);
    lv_obj_add_event_cb(item, on_click, LV_EVENT_CLICKED, (void*)(uintptr_t)index);

    lv_obj_t* lbl_primary = lv_label_create(item);
    lv_label_set_text(lbl_primary, primary_text);
    lv_obj_set_style_text_color(lbl_primary, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_primary, &lv_font_montserrat_30, 0);
    lv_label_set_long_mode(lbl_primary, LV_LABEL_LONG_DOT);
    lv_obj_set_width(lbl_primary, lv_pct(70));
    lv_obj_align(lbl_primary, LV_ALIGN_LEFT_MID, 0, 0);

    if (secondary_text && strlen(secondary_text) > 0) {
        lv_obj_t* lbl_secondary = lv_label_create(item);
        lv_label_set_text(lbl_secondary, secondary_text);
        lv_obj_set_style_text_color(lbl_secondary, COLOR_SECONDARY, 0);
        lv_obj_set_style_text_font(lbl_secondary, &lv_font_montserrat_24, 0);
        lv_obj_align(lbl_secondary, LV_ALIGN_RIGHT_MID, 0, 0);
    }

    return item;
}

// ============================================================================
// NOW PLAYING SCREEN
// ============================================================================

static void on_play_pause_click(lv_event_t* e) {
    // Send command to app
    if (g_command_callback) {
        g_command_callback("PLAY_PAUSE");
    }
    // Toggle local state immediately for responsive UI
    g_playback.is_playing = !g_playback.is_playing;
    update_now_playing_display();
}

static void on_prev_track_click(lv_event_t* e) {
    // Send command to app
    if (g_command_callback) {
        g_command_callback("PREV_SONG");
    }
    // App will send SONG_STARTED when the new song begins
}

static void on_next_track_click(lv_event_t* e) {
    // Send command to app
    if (g_command_callback) {
        g_command_callback("NEXT_SONG");
    }
    // App will send SONG_STARTED when the new song begins
}

static void on_shuffle_click(lv_event_t* e) {
    g_playback.shuffle_enabled = !g_playback.shuffle_enabled;
    update_now_playing_display();
}

static void on_library_btn_click(lv_event_t* e) {
    ui_show_library();
}

static void update_now_playing_display(void) {
    if (!g_np_song_title) return;

    // Determine song info source (BLE or hardcoded)
    const char* title = nullptr;
    const char* artist = nullptr;
    const char* album = nullptr;
    uint16_t duration = 0;

    if (g_using_ble_song) {
        title = g_ble_song_title;
        artist = g_ble_song_artist;
        album = g_ble_song_album;
        duration = g_ble_song_duration;
    } else if (g_playback.current_song) {
        title = g_playback.current_song->title;
        artist = g_playback.current_song->artist;
        album = g_playback.current_song->album;
        duration = g_playback.current_song->duration_sec;
    }

    if (title) {
        lv_label_set_text(g_np_song_title, title);
        lv_label_set_text(g_np_artist, artist ? artist : "---");
        lv_label_set_text(g_np_album, album ? album : "---");

        // Update progress
        if (duration > 0) {
            int32_t progress = (g_playback.progress_sec * 100) / duration;
            lv_bar_set_value(g_np_progress_bar, progress, LV_ANIM_OFF);
        }
        lv_label_set_text(g_np_time_current, format_duration(g_playback.progress_sec));
        lv_label_set_text(g_np_time_total, format_duration(duration));
    } else {
        lv_label_set_text(g_np_song_title, "No Song Selected");
        lv_label_set_text(g_np_artist, "---");
        lv_label_set_text(g_np_album, "---");
        lv_bar_set_value(g_np_progress_bar, 0, LV_ANIM_OFF);
        lv_label_set_text(g_np_time_current, "0:00");
        lv_label_set_text(g_np_time_total, "0:00");
    }

    // Update play/pause button
    lv_obj_t* play_lbl = lv_obj_get_child(g_np_btn_play, 0);
    if (play_lbl) {
        lv_label_set_text(play_lbl, g_playback.is_playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
    }

    // Update shuffle button
    if (g_playback.shuffle_enabled) {
        lv_obj_set_style_bg_color(g_np_btn_shuffle, COLOR_ACCENT, 0);
    } else {
        lv_obj_set_style_bg_color(g_np_btn_shuffle, COLOR_BUTTON_BG, 0);
    }
}

static void create_now_playing_screen(void) {
    lv_obj_t* old_screen = g_screen;
    g_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(g_screen, COLOR_BG, 0);

    // Header with Shuffle button (left) and Library button (right)
    lv_obj_t* header = lv_obj_create(g_screen);
    lv_obj_set_size(header, SCREEN_WIDTH, HEADER_HEIGHT);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, COLOR_HEADER_BG, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_all(header, 20, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Shuffle button (left side of header)
    g_np_btn_shuffle = lv_btn_create(header);
    lv_obj_set_size(g_np_btn_shuffle, 160, 80);
    lv_obj_align(g_np_btn_shuffle, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(g_np_btn_shuffle, COLOR_BUTTON_BG, 0);
    lv_obj_set_style_bg_color(g_np_btn_shuffle, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_add_event_cb(g_np_btn_shuffle, on_shuffle_click, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* lbl_shuffle = lv_label_create(g_np_btn_shuffle);
    lv_label_set_text(lbl_shuffle, LV_SYMBOL_SHUFFLE " Shuffle");
    lv_obj_set_style_text_color(lbl_shuffle, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_shuffle, &lv_font_montserrat_24, 0);
    lv_obj_center(lbl_shuffle);

    lv_obj_t* lbl_title = lv_label_create(header);
    lv_label_set_text(lbl_title, "Now Playing");
    lv_obj_set_style_text_color(lbl_title, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_30, 0);
    lv_obj_align(lbl_title, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* btn_library = lv_btn_create(header);
    lv_obj_set_size(btn_library, 200, 80);
    lv_obj_align(btn_library, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_library, COLOR_BUTTON_BG, 0);
    lv_obj_set_style_bg_color(btn_library, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_add_event_cb(btn_library, on_library_btn_click, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* lbl_lib = lv_label_create(btn_library);
    lv_label_set_text(lbl_lib, LV_SYMBOL_LIST " Library");
    lv_obj_set_style_text_color(lbl_lib, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_lib, &lv_font_montserrat_24, 0);
    lv_obj_center(lbl_lib);

    // Content area
    lv_obj_t* content = create_content_area();

    // Album art placeholder (left side)
    lv_obj_t* album_art = lv_obj_create(content);
    lv_obj_set_size(album_art, 200, 200);
    lv_obj_align(album_art, LV_ALIGN_LEFT_MID, 20, -20);
    lv_obj_set_style_bg_color(album_art, COLOR_ALBUM_ART, 0);
    lv_obj_set_style_border_width(album_art, 0, 0);
    lv_obj_set_style_radius(album_art, 10, 0);
    lv_obj_clear_flag(album_art, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* album_icon = lv_label_create(album_art);
    lv_label_set_text(album_icon, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_color(album_icon, COLOR_SECONDARY, 0);
    lv_obj_set_style_text_font(album_icon, &lv_font_montserrat_30, 0);
    lv_obj_center(album_icon);

    // Song info (right of album art)
    int info_x = 260;

    g_np_song_title = lv_label_create(content);
    lv_label_set_text(g_np_song_title, "No Song Selected");
    lv_obj_set_style_text_color(g_np_song_title, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(g_np_song_title, &lv_font_montserrat_30, 0);  // 25% bigger (24->30)
    lv_label_set_long_mode(g_np_song_title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(g_np_song_title, 480);
    lv_obj_set_pos(g_np_song_title, info_x, 15);

    g_np_artist = lv_label_create(content);
    lv_label_set_text(g_np_artist, "---");
    lv_obj_set_style_text_color(g_np_artist, COLOR_SECONDARY, 0);
    lv_obj_set_style_text_font(g_np_artist, &lv_font_montserrat_30, 0);  // 2x bigger (16->30)
    lv_obj_set_pos(g_np_artist, info_x, 55);

    g_np_album = lv_label_create(content);
    lv_label_set_text(g_np_album, "---");
    lv_obj_set_style_text_color(g_np_album, COLOR_SECONDARY, 0);
    lv_obj_set_style_text_font(g_np_album, &lv_font_montserrat_26, 0);  // 2x bigger (14->26)
    lv_obj_set_pos(g_np_album, info_x, 95);

    // Progress bar
    g_np_progress_bar = lv_bar_create(content);
    lv_obj_set_size(g_np_progress_bar, 480, 10);
    lv_obj_set_pos(g_np_progress_bar, info_x, 140);
    lv_bar_set_range(g_np_progress_bar, 0, 100);
    lv_bar_set_value(g_np_progress_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_np_progress_bar, COLOR_PROGRESS_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_np_progress_bar, COLOR_PROGRESS_FG, LV_PART_INDICATOR);
    lv_obj_set_style_radius(g_np_progress_bar, 5, LV_PART_MAIN);
    lv_obj_set_style_radius(g_np_progress_bar, 5, LV_PART_INDICATOR);

    // Time labels
    g_np_time_current = lv_label_create(content);
    lv_label_set_text(g_np_time_current, "0:00");
    lv_obj_set_style_text_color(g_np_time_current, COLOR_SECONDARY, 0);
    lv_obj_set_pos(g_np_time_current, info_x, 155);

    g_np_time_total = lv_label_create(content);
    lv_label_set_text(g_np_time_total, "0:00");
    lv_obj_set_style_text_color(g_np_time_total, COLOR_SECONDARY, 0);
    lv_obj_set_pos(g_np_time_total, info_x + 440, 155);

    // Playback controls (centered, no shuffle/repeat)
    int ctrl_y = 195;
    int ctrl_x = info_x + 140;  // Adjusted for centering 3 buttons
    int btn_size = 60;
    int btn_spacing = 90;

    // Previous button
    lv_obj_t* btn_prev = lv_btn_create(content);
    lv_obj_set_size(btn_prev, btn_size, btn_size);
    lv_obj_set_pos(btn_prev, ctrl_x, ctrl_y - 5);
    lv_obj_set_style_bg_color(btn_prev, COLOR_BUTTON_BG, 0);
    lv_obj_set_style_bg_color(btn_prev, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_prev, btn_size / 2, 0);
    lv_obj_add_event_cb(btn_prev, on_prev_track_click, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* lbl_prev = lv_label_create(btn_prev);
    lv_label_set_text(lbl_prev, LV_SYMBOL_PREV);
    lv_obj_set_style_text_color(lbl_prev, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_prev, &lv_font_montserrat_20, 0);
    lv_obj_center(lbl_prev);

    // Play/Pause button (larger)
    g_np_btn_play = lv_btn_create(content);
    lv_obj_set_size(g_np_btn_play, 80, 80);
    lv_obj_set_pos(g_np_btn_play, ctrl_x + btn_spacing, ctrl_y - 15);
    lv_obj_set_style_bg_color(g_np_btn_play, COLOR_ACCENT, 0);
    lv_obj_set_style_bg_color(g_np_btn_play, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_set_style_radius(g_np_btn_play, 40, 0);
    lv_obj_add_event_cb(g_np_btn_play, on_play_pause_click, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* lbl_play = lv_label_create(g_np_btn_play);
    lv_label_set_text(lbl_play, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_color(lbl_play, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_play, &lv_font_montserrat_24, 0);
    lv_obj_center(lbl_play);

    // Next button
    lv_obj_t* btn_next = lv_btn_create(content);
    lv_obj_set_size(btn_next, btn_size, btn_size);
    lv_obj_set_pos(btn_next, ctrl_x + btn_spacing * 2 + 20, ctrl_y - 5);
    lv_obj_set_style_bg_color(btn_next, COLOR_BUTTON_BG, 0);
    lv_obj_set_style_bg_color(btn_next, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_next, btn_size / 2, 0);
    lv_obj_add_event_cb(btn_next, on_next_track_click, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* lbl_next = lv_label_create(btn_next);
    lv_label_set_text(lbl_next, LV_SYMBOL_NEXT);
    lv_obj_set_style_text_color(lbl_next, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_next, &lv_font_montserrat_20, 0);
    lv_obj_center(lbl_next);

    update_now_playing_display();

    lv_scr_load(g_screen);
    if (old_screen != nullptr) {
        lv_obj_del(old_screen);
    }
    g_current_screen = SCREEN_NOW_PLAYING;
}

// ============================================================================
// LIBRARY SCREEN
// ============================================================================

static void on_playlists_btn_click(lv_event_t* e) {
    ui_show_playlists();
}

static void on_albums_btn_click(lv_event_t* e) {
    ui_show_albums();
}

static void on_artists_btn_click(lv_event_t* e) {
    ui_show_artists();
}

static void on_back_btn_click(lv_event_t* e) {
    // Navigate back based on current screen
    switch (g_current_screen) {
        case SCREEN_LIBRARY:
            ui_show_now_playing();
            break;
        case SCREEN_PLAYLISTS:
        case SCREEN_ALBUMS:
        case SCREEN_ARTISTS:
            ui_show_library();
            break;
        case SCREEN_PLAYLIST_DETAIL:
            ui_show_playlists();
            break;
        case SCREEN_ALBUM_DETAIL:
            if (g_selected_artist) {
                ui_show_artist_albums(g_selected_artist);
            } else {
                ui_show_albums();
            }
            break;
        case SCREEN_ARTIST_ALBUMS:
            ui_show_artists();
            break;
        default:
            ui_show_now_playing();
            break;
    }
}

static void on_now_playing_btn_click(lv_event_t* e) {
    ui_show_now_playing();
}

static void create_library_screen(void) {
    lv_obj_t* old_screen = g_screen;
    g_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(g_screen, COLOR_BG, 0);

    create_header("Library", true, true);

    lv_obj_t* content = create_content_area();

    // Three large buttons for navigation
    int btn_width = 220;
    int btn_height = 180;
    int spacing = 40;
    int start_x = (SCREEN_WIDTH - (3 * btn_width + 2 * spacing)) / 2;
    int btn_y = (CONTENT_HEIGHT - btn_height) / 2 - 20;

    // Playlists button
    lv_obj_t* btn_playlists = lv_btn_create(content);
    lv_obj_set_size(btn_playlists, btn_width, btn_height);
    lv_obj_set_pos(btn_playlists, start_x - 20, btn_y);
    lv_obj_set_style_bg_color(btn_playlists, COLOR_BUTTON_BG, 0);
    lv_obj_set_style_bg_color(btn_playlists, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_playlists, 15, 0);
    lv_obj_add_event_cb(btn_playlists, on_playlists_btn_click, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* icon_pl = lv_label_create(btn_playlists);
    lv_label_set_text(icon_pl, LV_SYMBOL_LIST);
    lv_obj_set_style_text_color(icon_pl, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(icon_pl, &lv_font_montserrat_30, 0);
    lv_obj_align(icon_pl, LV_ALIGN_CENTER, 0, -25);

    lv_obj_t* lbl_pl = lv_label_create(btn_playlists);
    lv_label_set_text(lbl_pl, "Playlists");
    lv_obj_set_style_text_color(lbl_pl, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_pl, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_pl, LV_ALIGN_CENTER, 0, 40);

    // Albums button
    lv_obj_t* btn_albums = lv_btn_create(content);
    lv_obj_set_size(btn_albums, btn_width, btn_height);
    lv_obj_set_pos(btn_albums, start_x + btn_width + spacing - 20, btn_y);
    lv_obj_set_style_bg_color(btn_albums, COLOR_BUTTON_BG, 0);
    lv_obj_set_style_bg_color(btn_albums, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_albums, 15, 0);
    lv_obj_add_event_cb(btn_albums, on_albums_btn_click, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* icon_al = lv_label_create(btn_albums);
    lv_label_set_text(icon_al, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_color(icon_al, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(icon_al, &lv_font_montserrat_30, 0);
    lv_obj_align(icon_al, LV_ALIGN_CENTER, 0, -25);

    lv_obj_t* lbl_al = lv_label_create(btn_albums);
    lv_label_set_text(lbl_al, "Albums");
    lv_obj_set_style_text_color(lbl_al, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_al, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_al, LV_ALIGN_CENTER, 0, 40);

    // Artists button
    lv_obj_t* btn_artists = lv_btn_create(content);
    lv_obj_set_size(btn_artists, btn_width, btn_height);
    lv_obj_set_pos(btn_artists, start_x + 2 * (btn_width + spacing) - 20, btn_y);
    lv_obj_set_style_bg_color(btn_artists, COLOR_BUTTON_BG, 0);
    lv_obj_set_style_bg_color(btn_artists, COLOR_BUTTON_PRESS, LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_artists, 15, 0);
    lv_obj_add_event_cb(btn_artists, on_artists_btn_click, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* icon_ar = lv_label_create(btn_artists);
    lv_label_set_text(icon_ar, LV_SYMBOL_SETTINGS);  // Using settings as person icon
    lv_obj_set_style_text_color(icon_ar, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(icon_ar, &lv_font_montserrat_30, 0);
    lv_obj_align(icon_ar, LV_ALIGN_CENTER, 0, -25);

    lv_obj_t* lbl_ar = lv_label_create(btn_artists);
    lv_label_set_text(lbl_ar, "Artists");
    lv_obj_set_style_text_color(lbl_ar, COLOR_PRIMARY, 0);
    lv_obj_set_style_text_font(lbl_ar, &lv_font_montserrat_20, 0);
    lv_obj_align(lbl_ar, LV_ALIGN_CENTER, 0, 40);

    lv_scr_load(g_screen);
    if (old_screen != nullptr) {
        lv_obj_del(old_screen);
    }
    g_current_screen = SCREEN_LIBRARY;
}

// ============================================================================
// PLAYLISTS SCREEN
// ============================================================================

static uint8_t get_playlists_count(void) {
    if (library_has_ble_data()) {
        return library_get_playlist_count();
    }
    return ALL_PLAYLISTS_COUNT;
}

static void on_playlist_click(lv_event_t* e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    uint8_t actual_index = g_list_page * ITEMS_PER_PAGE + index;

    // Save selection for next time
    library_set_last_playlist_index(actual_index);
    library_save_selections();

    if (library_has_ble_data()) {
        const BLEPlaylist* pl = library_get_playlist(actual_index);
        if (pl) {
            strncpy(g_selected_ble_playlist_id, pl->id, sizeof(g_selected_ble_playlist_id) - 1);
            ui_show_ble_playlist_detail(pl->id, pl->name);
        }
    } else if (actual_index < ALL_PLAYLISTS_COUNT) {
        ui_show_playlist_detail(ALL_PLAYLISTS[actual_index]);
    }
}

static void on_playlists_prev(lv_event_t* e) {
    uint8_t count = get_playlists_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;

    if (g_list_page > 0) {
        g_list_page--;
    } else {
        // Wrap to last page
        g_list_page = total_pages - 1;
    }
    create_playlists_screen();
}

static void on_playlists_next(lv_event_t* e) {
    uint8_t count = get_playlists_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;

    if (g_list_page < total_pages - 1) {
        g_list_page++;
    } else {
        // Wrap to first page
        g_list_page = 0;
    }
    create_playlists_screen();
}

static void create_playlists_screen(void) {
    lv_obj_t* old_screen = g_screen;
    g_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(g_screen, COLOR_BG, 0);

    create_header("Playlists", true, true);

    uint8_t count = get_playlists_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;
    create_side_navigation(g_list_page, total_pages, on_playlists_prev, on_playlists_next);

    lv_obj_t* content = create_list_content_area();
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content, LIST_ITEM_SPACING, 0);

    uint8_t start_idx = g_list_page * ITEMS_PER_PAGE;
    uint8_t end_idx = start_idx + ITEMS_PER_PAGE;
    if (end_idx > count) end_idx = count;

    for (uint8_t i = start_idx; i < end_idx; i++) {
        static char subtitle[32];
        const char* name;
        uint16_t song_count;

        if (library_has_ble_data()) {
            const BLEPlaylist* pl = library_get_playlist(i);
            name = pl ? pl->name : "Unknown";
            song_count = pl ? pl->song_count : 0;
        } else {
            const Playlist* pl = ALL_PLAYLISTS[i];
            name = pl->name;
            song_count = pl->song_count;
        }

        snprintf(subtitle, sizeof(subtitle), "%d songs", song_count);
        create_list_item(content, name, subtitle, i - start_idx, on_playlist_click);
    }

    lv_scr_load(g_screen);
    if (old_screen != nullptr) {
        lv_obj_del(old_screen);
    }
    g_current_screen = SCREEN_PLAYLISTS;
}

// ============================================================================
// ALBUMS SCREEN
// ============================================================================

static uint8_t get_albums_count(void) {
    if (library_has_ble_data()) {
        return library_get_album_count();
    }
    return ALL_ALBUMS_COUNT;
}

static void on_album_click(lv_event_t* e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    uint8_t actual_index = g_list_page * ITEMS_PER_PAGE + index;

    // Save selection for next time
    library_set_last_album_index(actual_index);
    library_save_selections();

    if (library_has_ble_data()) {
        const BLEAlbum* album = library_get_album(actual_index);
        if (album) {
            strncpy(g_selected_ble_album_id, album->id, sizeof(g_selected_ble_album_id) - 1);
            g_selected_ble_artist_id[0] = '\0';  // Not from artist view
            ui_show_ble_album_detail(album->id, album->name);
        }
    } else if (actual_index < ALL_ALBUMS_COUNT) {
        g_selected_artist = nullptr;  // Not from artist view
        ui_show_album_detail(ALL_ALBUMS[actual_index]);
    }
}

static void on_albums_prev(lv_event_t* e) {
    uint8_t count = get_albums_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;

    if (g_list_page > 0) {
        g_list_page--;
    } else {
        // Wrap to last page
        g_list_page = total_pages - 1;
    }
    create_albums_screen();
}

static void on_albums_next(lv_event_t* e) {
    uint8_t count = get_albums_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;

    if (g_list_page < total_pages - 1) {
        g_list_page++;
    } else {
        // Wrap to first page
        g_list_page = 0;
    }
    create_albums_screen();
}

static void create_albums_screen(void) {
    lv_obj_t* old_screen = g_screen;
    g_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(g_screen, COLOR_BG, 0);

    create_header("Albums", true, true);

    uint8_t count = get_albums_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;
    create_side_navigation(g_list_page, total_pages, on_albums_prev, on_albums_next);

    lv_obj_t* content = create_list_content_area();
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content, LIST_ITEM_SPACING, 0);

    uint8_t start_idx = g_list_page * ITEMS_PER_PAGE;
    uint8_t end_idx = start_idx + ITEMS_PER_PAGE;
    if (end_idx > count) end_idx = count;

    for (uint8_t i = start_idx; i < end_idx; i++) {
        const char* name;
        const char* artist;

        if (library_has_ble_data()) {
            const BLEAlbum* album = library_get_album(i);
            name = album ? album->name : "Unknown";
            artist = album ? album->artist : "Unknown";
        } else {
            const Album* album = ALL_ALBUMS[i];
            name = album->name;
            artist = album->artist;
        }

        create_list_item(content, name, artist, i - start_idx, on_album_click);
    }

    lv_scr_load(g_screen);
    if (old_screen != nullptr) {
        lv_obj_del(old_screen);
    }
    g_current_screen = SCREEN_ALBUMS;
}

// ============================================================================
// ARTISTS SCREEN
// ============================================================================

static uint8_t get_artists_count(void) {
    if (library_has_ble_data()) {
        return library_get_artist_count();
    }
    return ALL_ARTISTS_COUNT;
}

static void on_artist_click(lv_event_t* e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    uint8_t actual_index = g_list_page * ITEMS_PER_PAGE + index;

    // Save selection for next time
    library_set_last_artist_index(actual_index);
    library_save_selections();

    if (library_has_ble_data()) {
        const BLEArtist* artist = library_get_artist(actual_index);
        if (artist) {
            strncpy(g_selected_ble_artist_id, artist->id, sizeof(g_selected_ble_artist_id) - 1);
            ui_show_ble_artist_albums(artist->id, artist->name);
        }
    } else if (actual_index < ALL_ARTISTS_COUNT) {
        ui_show_artist_albums(ALL_ARTISTS[actual_index]);
    }
}

static void on_artists_prev(lv_event_t* e) {
    uint8_t count = get_artists_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;

    if (g_list_page > 0) {
        g_list_page--;
    } else {
        // Wrap to last page
        g_list_page = total_pages - 1;
    }
    create_artists_screen();
}

static void on_artists_next(lv_event_t* e) {
    uint8_t count = get_artists_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;

    if (g_list_page < total_pages - 1) {
        g_list_page++;
    } else {
        // Wrap to first page
        g_list_page = 0;
    }
    create_artists_screen();
}

static void create_artists_screen(void) {
    lv_obj_t* old_screen = g_screen;
    g_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(g_screen, COLOR_BG, 0);

    create_header("Artists", true, true);

    uint8_t count = get_artists_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;
    create_side_navigation(g_list_page, total_pages, on_artists_prev, on_artists_next);

    lv_obj_t* content = create_list_content_area();
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content, LIST_ITEM_SPACING, 0);

    uint8_t start_idx = g_list_page * ITEMS_PER_PAGE;
    uint8_t end_idx = start_idx + ITEMS_PER_PAGE;
    if (end_idx > count) end_idx = count;

    for (uint8_t i = start_idx; i < end_idx; i++) {
        static char subtitle[32];
        const char* name;
        uint8_t album_count;

        if (library_has_ble_data()) {
            const BLEArtist* artist = library_get_artist(i);
            name = artist ? artist->name : "Unknown";
            album_count = artist ? artist->album_count : 0;
        } else {
            const Artist* artist = ALL_ARTISTS[i];
            name = artist->name;
            album_count = artist->album_count;
        }

        snprintf(subtitle, sizeof(subtitle), "%d albums", album_count);
        create_list_item(content, name, subtitle, i - start_idx, on_artist_click);
    }

    lv_scr_load(g_screen);
    if (old_screen != nullptr) {
        lv_obj_del(old_screen);
    }
    g_current_screen = SCREEN_ARTISTS;
}

// ============================================================================
// PLAYLIST DETAIL SCREEN
// ============================================================================

static void on_playlist_song_click(lv_event_t* e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    uint8_t actual_index = g_list_page * ITEMS_PER_PAGE + index;
    if (g_selected_playlist && actual_index < g_selected_playlist->song_count) {
        g_playback.current_song = g_selected_playlist->songs[actual_index];
        g_playback.progress_sec = 0;
        g_playback.is_playing = true;
        ui_show_now_playing();
    }
}

static void on_playlist_detail_prev(lv_event_t* e) {
    if (g_list_page > 0) {
        g_list_page--;
        create_playlist_detail_screen(g_selected_playlist);
    }
}

static void on_playlist_detail_next(lv_event_t* e) {
    if (g_selected_playlist) {
        uint8_t total_pages = (g_selected_playlist->song_count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
        if (g_list_page < total_pages - 1) {
            g_list_page++;
            create_playlist_detail_screen(g_selected_playlist);
        }
    }
}

static void create_playlist_detail_screen(const Playlist* playlist) {
    lv_obj_t* old_screen = g_screen;
    g_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(g_screen, COLOR_BG, 0);

    create_header(playlist->name, true, true);

    uint8_t total_pages = (playlist->song_count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;
    create_side_navigation(g_list_page, total_pages, on_playlist_detail_prev, on_playlist_detail_next);

    lv_obj_t* content = create_list_content_area();
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content, LIST_ITEM_SPACING, 0);

    uint8_t start_idx = g_list_page * ITEMS_PER_PAGE;
    uint8_t end_idx = start_idx + ITEMS_PER_PAGE;
    if (end_idx > playlist->song_count) end_idx = playlist->song_count;

    for (uint8_t i = start_idx; i < end_idx; i++) {
        const Song* song = playlist->songs[i];
        static char subtitle[64];
        snprintf(subtitle, sizeof(subtitle), "%s - %s", song->artist, format_duration(song->duration_sec));
        create_list_item(content, song->title, subtitle, i - start_idx, on_playlist_song_click);
    }

    lv_scr_load(g_screen);
    if (old_screen != nullptr) {
        lv_obj_del(old_screen);
    }
    g_current_screen = SCREEN_PLAYLIST_DETAIL;
}

// ============================================================================
// ALBUM DETAIL SCREEN
// ============================================================================

static void on_album_song_click(lv_event_t* e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    uint8_t actual_index = g_list_page * ITEMS_PER_PAGE + index;
    if (g_selected_album && actual_index < g_selected_album->song_count) {
        g_playback.current_song = g_selected_album->songs[actual_index];
        g_playback.progress_sec = 0;
        g_playback.is_playing = true;
        ui_show_now_playing();
    }
}

static void on_album_detail_prev(lv_event_t* e) {
    if (g_list_page > 0) {
        g_list_page--;
        create_album_detail_screen(g_selected_album);
    }
}

static void on_album_detail_next(lv_event_t* e) {
    if (g_selected_album) {
        uint8_t total_pages = (g_selected_album->song_count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
        if (g_list_page < total_pages - 1) {
            g_list_page++;
            create_album_detail_screen(g_selected_album);
        }
    }
}

static void create_album_detail_screen(const Album* album) {
    lv_obj_t* old_screen = g_screen;
    g_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(g_screen, COLOR_BG, 0);

    static char header_text[64];
    snprintf(header_text, sizeof(header_text), "%s", album->name);
    create_header(header_text, true, true);

    uint8_t total_pages = (album->song_count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;
    create_side_navigation(g_list_page, total_pages, on_album_detail_prev, on_album_detail_next);

    lv_obj_t* content = create_list_content_area();
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content, LIST_ITEM_SPACING, 0);

    uint8_t start_idx = g_list_page * ITEMS_PER_PAGE;
    uint8_t end_idx = start_idx + ITEMS_PER_PAGE;
    if (end_idx > album->song_count) end_idx = album->song_count;

    for (uint8_t i = start_idx; i < end_idx; i++) {
        const Song* song = album->songs[i];
        create_list_item(content, song->title, format_duration(song->duration_sec),
                        i - start_idx, on_album_song_click);
    }

    lv_scr_load(g_screen);
    if (old_screen != nullptr) {
        lv_obj_del(old_screen);
    }
    g_current_screen = SCREEN_ALBUM_DETAIL;
}

// ============================================================================
// ARTIST ALBUMS SCREEN
// ============================================================================

static void on_artist_album_click(lv_event_t* e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    uint8_t actual_index = g_list_page * ITEMS_PER_PAGE + index;
    if (g_selected_artist && actual_index < g_selected_artist->album_count) {
        ui_show_album_detail(g_selected_artist->albums[actual_index]);
    }
}

static void on_artist_albums_prev(lv_event_t* e) {
    if (g_list_page > 0) {
        g_list_page--;
        create_artist_albums_screen(g_selected_artist);
    }
}

static void on_artist_albums_next(lv_event_t* e) {
    if (g_selected_artist) {
        uint8_t total_pages = (g_selected_artist->album_count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
        if (g_list_page < total_pages - 1) {
            g_list_page++;
            create_artist_albums_screen(g_selected_artist);
        }
    }
}

static void create_artist_albums_screen(const Artist* artist) {
    lv_obj_t* old_screen = g_screen;
    g_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(g_screen, COLOR_BG, 0);

    create_header(artist->name, true, true);

    uint8_t total_pages = (artist->album_count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;
    create_side_navigation(g_list_page, total_pages, on_artist_albums_prev, on_artist_albums_next);

    lv_obj_t* content = create_list_content_area();
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content, LIST_ITEM_SPACING, 0);

    uint8_t start_idx = g_list_page * ITEMS_PER_PAGE;
    uint8_t end_idx = start_idx + ITEMS_PER_PAGE;
    if (end_idx > artist->album_count) end_idx = artist->album_count;

    for (uint8_t i = start_idx; i < end_idx; i++) {
        const Album* album = artist->albums[i];
        static char subtitle[32];
        snprintf(subtitle, sizeof(subtitle), "%d songs", album->song_count);
        create_list_item(content, album->name, subtitle, i - start_idx, on_artist_album_click);
    }

    lv_scr_load(g_screen);
    if (old_screen != nullptr) {
        lv_obj_del(old_screen);
    }
    g_current_screen = SCREEN_ARTIST_ALBUMS;
}

// ============================================================================
// PUBLIC API
// ============================================================================

void ui_init(void) {
    // Set default song
    if (ALL_SONGS_COUNT > 0) {
        g_playback.current_song = ALL_SONGS[0];
    }

    // Create and show the Now Playing screen
    create_now_playing_screen();
}

void ui_show_screen(screen_t screen) {
    switch (screen) {
        case SCREEN_NOW_PLAYING:
            ui_show_now_playing();
            break;
        case SCREEN_LIBRARY:
            ui_show_library();
            break;
        case SCREEN_PLAYLISTS:
            ui_show_playlists();
            break;
        case SCREEN_ALBUMS:
            ui_show_albums();
            break;
        case SCREEN_ARTISTS:
            ui_show_artists();
            break;
        default:
            ui_show_now_playing();
            break;
    }
}

void ui_show_now_playing(void) {
    g_np_song_title = nullptr;  // Reset widget pointers
    create_now_playing_screen();
}

void ui_show_library(void) {
    create_library_screen();
}

void ui_show_playlists(void) {
    // Start at page containing last selected item
    uint8_t last_index = library_get_last_playlist_index();
    g_list_page = last_index / ITEMS_PER_PAGE;
    create_playlists_screen();
}

void ui_show_albums(void) {
    // Start at page containing last selected item
    uint8_t last_index = library_get_last_album_index();
    g_list_page = last_index / ITEMS_PER_PAGE;
    create_albums_screen();
}

void ui_show_artists(void) {
    // Start at page containing last selected item
    uint8_t last_index = library_get_last_artist_index();
    g_list_page = last_index / ITEMS_PER_PAGE;
    create_artists_screen();
}

void ui_show_playlist_detail(const Playlist* playlist) {
    g_selected_playlist = playlist;
    g_list_page = 0;
    create_playlist_detail_screen(playlist);
}

void ui_show_album_detail(const Album* album) {
    g_selected_album = album;
    g_list_page = 0;
    create_album_detail_screen(album);
}

void ui_show_artist_albums(const Artist* artist) {
    g_selected_artist = artist;
    g_list_page = 0;
    create_artist_albums_screen(artist);
}

void ui_set_current_song(const Song* song) {
    g_using_ble_song = false;
    g_playback.current_song = song;
    g_playback.progress_sec = 0;
    if (g_current_screen == SCREEN_NOW_PLAYING) {
        update_now_playing_display();
    }
}

void ui_set_song_info(const char* title, const char* artist, const char* album, uint16_t duration_sec) {
    g_using_ble_song = true;
    g_playback.current_song = nullptr;

    // Copy strings to buffers
    if (title) {
        strncpy(g_ble_song_title, title, sizeof(g_ble_song_title) - 1);
        g_ble_song_title[sizeof(g_ble_song_title) - 1] = '\0';
    } else {
        g_ble_song_title[0] = '\0';
    }

    if (artist) {
        strncpy(g_ble_song_artist, artist, sizeof(g_ble_song_artist) - 1);
        g_ble_song_artist[sizeof(g_ble_song_artist) - 1] = '\0';
    } else {
        g_ble_song_artist[0] = '\0';
    }

    if (album) {
        strncpy(g_ble_song_album, album, sizeof(g_ble_song_album) - 1);
        g_ble_song_album[sizeof(g_ble_song_album) - 1] = '\0';
    } else {
        g_ble_song_album[0] = '\0';
    }

    g_ble_song_duration = duration_sec;

    if (g_current_screen == SCREEN_NOW_PLAYING) {
        update_now_playing_display();
    }
}

void ui_set_playing(bool playing) {
    g_playback.is_playing = playing;
    if (g_current_screen == SCREEN_NOW_PLAYING) {
        update_now_playing_display();
    }
}

void ui_set_progress(uint16_t progress_sec) {
    g_playback.progress_sec = progress_sec;
    if (g_current_screen == SCREEN_NOW_PLAYING) {
        update_now_playing_display();
    }
}

void ui_set_shuffle(bool enabled) {
    g_playback.shuffle_enabled = enabled;
    if (g_current_screen == SCREEN_NOW_PLAYING) {
        update_now_playing_display();
    }
}

void ui_set_repeat(bool enabled) {
    g_playback.repeat_enabled = enabled;
    if (g_current_screen == SCREEN_NOW_PLAYING) {
        update_now_playing_display();
    }
}

const playback_state_t* ui_get_playback_state(void) {
    return &g_playback;
}

void ui_update(void) {
    // Can be called periodically to update progress animation
    // For now, just refreshes the display if on Now Playing screen
    if (g_current_screen == SCREEN_NOW_PLAYING && g_np_progress_bar) {
        update_now_playing_display();
    }
}

void ui_set_query_callback(UIQueryCallback callback) {
    g_query_callback = (QueryCallback)callback;
}

void ui_set_play_callback(UIPlayCallback callback) {
    g_play_callback = (PlayCallback)callback;
}

void ui_set_command_callback(UICommandCallback callback) {
    g_command_callback = (CommandCallback)callback;
}

// ============================================================================
// BLE DETAIL SCREENS
// ============================================================================

void ui_show_ble_playlist_detail(const char* playlist_id, const char* name) {
    strncpy(g_ble_detail_id, playlist_id, sizeof(g_ble_detail_id) - 1);
    strncpy(g_ble_detail_name, name, sizeof(g_ble_detail_name) - 1);
    strncpy(g_ble_detail_type, "playlist", sizeof(g_ble_detail_type) - 1);
    g_list_page = 0;

    // Request songs from app
    if (g_query_callback) {
        g_query_callback("QUERY_PLAYLIST_SONGS", playlist_id);
    }

    // Show songs screen (will show loading or empty until data arrives)
    create_ble_songs_screen();
}

void ui_show_ble_album_detail(const char* album_id, const char* name) {
    strncpy(g_ble_detail_id, album_id, sizeof(g_ble_detail_id) - 1);
    strncpy(g_ble_detail_name, name, sizeof(g_ble_detail_name) - 1);
    strncpy(g_ble_detail_type, "album", sizeof(g_ble_detail_type) - 1);
    g_list_page = 0;

    // Request songs from app
    if (g_query_callback) {
        g_query_callback("QUERY_ALBUM_SONGS", album_id);
    }

    create_ble_songs_screen();
}

void ui_show_ble_artist_albums(const char* artist_id, const char* name) {
    strncpy(g_ble_detail_id, artist_id, sizeof(g_ble_detail_id) - 1);
    strncpy(g_ble_detail_name, name, sizeof(g_ble_detail_name) - 1);
    strncpy(g_ble_detail_type, "artist", sizeof(g_ble_detail_type) - 1);
    g_list_page = 0;

    // Request songs from app
    if (g_query_callback) {
        g_query_callback("QUERY_ARTIST_SONGS", artist_id);
    }

    create_ble_songs_screen();
}

void ui_show_ble_songs(void) {
    // Called when songs data arrives - refresh the screen
    create_ble_songs_screen();
}

// BLE songs screen - shows songs from library_data
static void on_ble_song_click(lv_event_t* e) {
    uint8_t index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    uint8_t actual_index = g_list_page * ITEMS_PER_PAGE + index;
    const BLESong* song = library_get_song(actual_index);

    if (song) {
        // Send play command to app
        if (g_play_callback) {
            const char* context = library_get_song_context_type();
            const char* context_id = library_get_song_context_id();
            // Use context if available, otherwise pass NULL
            if (context && strlen(context) > 0) {
                g_play_callback(song->id, context, context_id, actual_index);
            } else {
                g_play_callback(song->id, nullptr, nullptr, actual_index);
            }
        }

        // Update UI immediately (will be updated again when app sends SONG_STARTED)
        ui_set_song_info(song->title, song->artist, song->album, song->duration_sec);
        ui_set_playing(true);
        ui_set_progress(0);
        ui_show_now_playing();
    }
}

static void on_ble_songs_prev(lv_event_t* e) {
    uint8_t count = library_get_song_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;

    if (g_list_page > 0) {
        g_list_page--;
    } else {
        // Wrap to last page
        g_list_page = total_pages - 1;
    }
    create_ble_songs_screen();
}

static void on_ble_songs_next(lv_event_t* e) {
    uint8_t count = library_get_song_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;

    if (g_list_page < total_pages - 1) {
        g_list_page++;
    } else {
        // Wrap to first page
        g_list_page = 0;
    }
    create_ble_songs_screen();
}

static void create_ble_songs_screen(void) {
    lv_obj_t* old_screen = g_screen;
    g_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(g_screen, COLOR_BG, 0);

    create_header(g_ble_detail_name, true, true);

    uint8_t count = library_get_song_count();
    uint8_t total_pages = (count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    if (total_pages == 0) total_pages = 1;
    create_side_navigation(g_list_page, total_pages, on_ble_songs_prev, on_ble_songs_next);

    lv_obj_t* content = create_list_content_area();
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content, LIST_ITEM_SPACING, 0);

    if (count == 0) {
        // Show loading message
        lv_obj_t* lbl = lv_label_create(content);
        lv_label_set_text(lbl, "Loading...");
        lv_obj_set_style_text_color(lbl, COLOR_SECONDARY, 0);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, 0);
    } else {
        uint8_t start_idx = g_list_page * ITEMS_PER_PAGE;
        uint8_t end_idx = start_idx + ITEMS_PER_PAGE;
        if (end_idx > count) end_idx = count;

        for (uint8_t i = start_idx; i < end_idx; i++) {
            const BLESong* song = library_get_song(i);
            if (song) {
                static char subtitle[64];
                snprintf(subtitle, sizeof(subtitle), "%s", song->artist);
                create_list_item(content, song->title, subtitle, i - start_idx, on_ble_song_click);
            }
        }
    }

    lv_scr_load(g_screen);
    if (old_screen != nullptr) {
        lv_obj_del(old_screen);
    }

    // Set screen type based on context
    if (strcmp(g_ble_detail_type, "playlist") == 0) {
        g_current_screen = SCREEN_PLAYLIST_DETAIL;
    } else if (strcmp(g_ble_detail_type, "album") == 0) {
        g_current_screen = SCREEN_ALBUM_DETAIL;
    } else {
        g_current_screen = SCREEN_ARTIST_ALBUMS;
    }
}
