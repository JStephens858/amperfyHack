#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <ArduinoJson.h>

#include <lvgl.h>
#include "lvgl_v8_port.h"
#include "ui.h"
#include "bluetooth.h"
#include "library_data.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

// Flag to send initial queries after connection
static bool g_should_query_library = false;
static unsigned long g_connection_time = 0;
static const unsigned long QUERY_DELAY_MS = 2000;  // Wait 2 seconds for app to be ready

// Send a query message to the app
void send_query(const char* query_type, const char* id = nullptr) {
    StaticJsonDocument<256> doc;
    doc["type"] = query_type;
    doc["timestamp"] = millis() / 1000.0;

    if (id != nullptr && strlen(id) > 0) {
        JsonObject payload = doc.createNestedObject("payload");
        if (strcmp(query_type, "QUERY_PLAYLIST_SONGS") == 0) {
            payload["playlistId"] = id;
        } else if (strcmp(query_type, "QUERY_ALBUM_SONGS") == 0) {
            payload["albumId"] = id;
        } else if (strcmp(query_type, "QUERY_ARTIST_SONGS") == 0) {
            payload["artistId"] = id;
        }
    }

    char buffer[256];
    serializeJson(doc, buffer, sizeof(buffer));
    bluetooth_send(buffer);
    Serial.print("[Main] Sent query: ");
    Serial.println(buffer);
}

// UI query callback - called when UI needs data from app
void on_ui_query(const char* query_type, const char* id) {
    // Songs are now cleared when we receive the first page of response
    send_query(query_type, id);
}

// UI play callback - called when user taps a song to play
void on_ui_play(const char* song_id, const char* context, const char* context_id, int song_index) {
    StaticJsonDocument<256> doc;
    doc["type"] = "PLAY_SONG";
    doc["timestamp"] = millis() / 1000.0;

    JsonObject payload = doc.createNestedObject("payload");
    payload["songId"] = song_id;
    if (context != nullptr && strlen(context) > 0) {
        payload["context"] = context;
        payload["contextId"] = context_id;
    }
    payload["songIndex"] = song_index;

    char buffer[256];
    serializeJson(doc, buffer, sizeof(buffer));
    bluetooth_send(buffer);

    Serial.print("[Main] Sent play command: ");
    Serial.println(buffer);
}

// UI command callback - called for playback control (play/pause, next, prev)
void on_ui_command(const char* command) {
    StaticJsonDocument<128> doc;
    doc["type"] = command;
    doc["timestamp"] = millis() / 1000.0;

    char buffer[128];
    serializeJson(doc, buffer, sizeof(buffer));
    bluetooth_send(buffer);

    Serial.print("[Main] Sent command: ");
    Serial.println(buffer);
}

// Send initial library queries
void send_library_queries() {
    Serial.println("[Main] Requesting library data...");
    send_query("QUERY_PLAYLISTS");
    delay(50);
    send_query("QUERY_ARTISTS");
    delay(50);
    send_query("QUERY_ALBUMS");
}

// Bluetooth connection callback
void on_ble_connection(bool connected) {
    Serial.print("[Main] BLE connection: ");
    Serial.println(connected ? "connected" : "disconnected");

    if (connected) {
        // Record connection time - we'll send queries after a delay
        g_connection_time = millis();
        g_should_query_library = true;
        Serial.println("[Main] Will query library in 2 seconds...");
    } else {
        // Clear library data on disconnect
        g_should_query_library = false;
        library_data_clear();
    }
}

// Handle SONG_STARTED message
void handle_song_started(JsonObject& payload) {
    const char* title = payload["title"] | "Unknown";
    const char* artist = payload["artist"] | "Unknown Artist";
    const char* album = payload["album"] | "Unknown Album";
    float duration = payload["duration"] | 0.0f;

    Serial.print("[Main] Song started: ");
    Serial.print(title);
    Serial.print(" - ");
    Serial.println(artist);

    lvgl_port_lock(-1);
    ui_set_song_info(title, artist, album, (uint16_t)duration);
    ui_set_playing(true);
    ui_set_progress(0);
    lvgl_port_unlock();
}

// Handle SONG_STOPPED message
void handle_song_stopped(JsonObject& payload) {
    Serial.println("[Main] Song stopped");

    lvgl_port_lock(-1);
    ui_set_playing(false);
    lvgl_port_unlock();
}

// Handle PLAYBACK_PROGRESS message
void handle_playback_progress(JsonObject& payload) {
    float elapsedTime = payload["elapsedTime"] | 0.0f;
    bool isPlaying = payload["isPlaying"] | false;

    lvgl_port_lock(-1);
    ui_set_progress((uint16_t)elapsedTime);
    ui_set_playing(isPlaying);
    lvgl_port_unlock();
}

// Handle PLAYLISTS_RESPONSE message
void handle_playlists_response(JsonObject& payload) {
    int page = payload["page"] | 1;
    int totalPages = payload["totalPages"] | 1;
    JsonArray playlists = payload["playlists"];

    Serial.print("[Main] Received playlists page ");
    Serial.print(page);
    Serial.print("/");
    Serial.print(totalPages);
    Serial.print(" (");
    Serial.print(playlists.size());
    Serial.println(" items)");

    // Only clear on first page
    if (page == 1) {
        library_clear_playlists();
    }

    for (JsonObject pl : playlists) {
        const char* id = pl["id"] | "";
        const char* name = pl["name"] | "Unknown";
        uint16_t songCount = pl["songCount"] | 0;
        library_add_playlist(id, name, songCount);
    }

    if (page == totalPages) {
        Serial.print("[Main] All playlists received, total: ");
        Serial.println(library_get_playlist_count());
    }
}

// Handle ARTISTS_RESPONSE message
void handle_artists_response(JsonObject& payload) {
    int page = payload["page"] | 1;
    int totalPages = payload["totalPages"] | 1;
    JsonArray artists = payload["artists"];

    Serial.print("[Main] Received artists page ");
    Serial.print(page);
    Serial.print("/");
    Serial.print(totalPages);
    Serial.print(" (");
    Serial.print(artists.size());
    Serial.println(" items)");

    // Only clear on first page
    if (page == 1) {
        library_clear_artists();
    }

    for (JsonObject artist : artists) {
        const char* id = artist["id"] | "";
        const char* name = artist["name"] | "Unknown";
        uint8_t albumCount = artist["albumCount"] | 0;
        uint16_t songCount = artist["songCount"] | 0;
        library_add_artist(id, name, albumCount, songCount);
    }

    if (page == totalPages) {
        Serial.print("[Main] All artists received, total: ");
        Serial.println(library_get_artist_count());
    }
}

// Handle ALBUMS_RESPONSE message
void handle_albums_response(JsonObject& payload) {
    int page = payload["page"] | 1;
    int totalPages = payload["totalPages"] | 1;
    JsonArray albums = payload["albums"];

    Serial.print("[Main] Received albums page ");
    Serial.print(page);
    Serial.print("/");
    Serial.print(totalPages);
    Serial.print(" (");
    Serial.print(albums.size());
    Serial.println(" items)");

    // Only clear on first page
    if (page == 1) {
        library_clear_albums();
    }

    for (JsonObject album : albums) {
        const char* id = album["id"] | "";
        const char* name = album["name"] | "Unknown";
        const char* artist = album["artist"] | "Unknown";
        uint8_t songCount = album["songCount"] | 0;
        uint16_t year = album["year"] | 0;
        library_add_album(id, name, artist, songCount, year);
    }

    if (page == totalPages) {
        Serial.print("[Main] All albums received, total: ");
        Serial.println(library_get_album_count());
    }
}

// Handle SONGS_RESPONSE message
void handle_songs_response(JsonObject& payload) {
    int page = payload["page"] | 1;
    int totalPages = payload["totalPages"] | 1;
    JsonArray songs = payload["songs"];

    Serial.print("[Main] Received songs page ");
    Serial.print(page);
    Serial.print("/");
    Serial.print(totalPages);
    Serial.print(" (");
    Serial.print(songs.size());
    Serial.println(" items)");

    // Only clear and set context on first page
    if (page == 1) {
        library_clear_songs();
        const char* context = payload["context"] | "";
        const char* contextId = payload["contextId"] | "";
        library_set_song_context(context, contextId);
    }

    for (JsonObject song : songs) {
        const char* id = song["id"] | "";
        const char* title = song["title"] | "Unknown";
        const char* artist = song["artist"] | "Unknown";
        const char* album = song["album"] | "Unknown";
        float duration = song["duration"] | 0.0f;
        uint8_t trackNumber = song["trackNumber"] | 0;
        library_add_song(id, title, artist, album, (uint16_t)duration, trackNumber);
    }

    // Only refresh UI after last page
    if (page == totalPages) {
        Serial.print("[Main] All songs received, total: ");
        Serial.println(library_get_song_count());

        lvgl_port_lock(-1);
        ui_show_ble_songs();
        lvgl_port_unlock();
    }
}

// Bluetooth data callback - receives JSON from Amperfy app
void on_ble_data(const char* data, size_t length) {
    // Print raw data received
    Serial.println("\n========== BLE DATA RECEIVED ==========");
    Serial.print("Length: ");
    Serial.print(length);
    Serial.println(" bytes");
    Serial.println("Raw JSON:");
    Serial.println(data);
    Serial.println("========================================\n");

    // Parse JSON - use larger buffer for library responses
    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, data, length);

    if (error) {
        Serial.print("[Main] JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }

    const char* type = doc["type"] | "";
    JsonObject payload = doc["payload"];

    if (strcmp(type, "SONG_STARTED") == 0) {
        handle_song_started(payload);
    } else if (strcmp(type, "SONG_STOPPED") == 0) {
        handle_song_stopped(payload);
    } else if (strcmp(type, "PLAYBACK_PROGRESS") == 0) {
        handle_playback_progress(payload);
    } else if (strcmp(type, "PLAYLISTS_RESPONSE") == 0) {
        handle_playlists_response(payload);
    } else if (strcmp(type, "ARTISTS_RESPONSE") == 0) {
        handle_artists_response(payload);
    } else if (strcmp(type, "ALBUMS_RESPONSE") == 0) {
        handle_albums_response(payload);
    } else if (strcmp(type, "SONGS_RESPONSE") == 0) {
        handle_songs_response(payload);
    } else {
        Serial.print("[Main] Unknown message type: ");
        Serial.println(type);
    }
}

void setup()
{
    Serial.begin(115200);

    Serial.println("Initializing board");
    Board *board = new Board();
    board->init();

    #if LVGL_PORT_AVOID_TEARING_MODE
    auto lcd = board->getLCD();
    // When avoid tearing function is enabled, the frame buffer number should be set in the board driver
    lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    /**
     * As the anti-tearing feature typically consumes more PSRAM bandwidth, for the ESP32-S3, we need to utilize the
     * "bounce buffer" functionality to enhance the RGB data bandwidth.
     * This feature will consume `bounce_buffer_size * bytes_per_pixel * 2` of SRAM memory.
     */
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    }
#endif
#endif
    assert(board->begin());

    Serial.println("Initializing LVGL");
    lvgl_port_init(board->getLCD(), board->getTouch());

    Serial.println("Creating UI");
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);

    /* Initialize the car music player UI */
    ui_init();

    /* Set up callbacks */
    ui_set_query_callback(on_ui_query);
    ui_set_play_callback(on_ui_play);
    ui_set_command_callback(on_ui_command);

    /* Release the mutex */
    lvgl_port_unlock();

    /* Initialize library data storage */
    library_data_init();
    library_load_selections();  // Load last selected indices from NVS

    /* Initialize Bluetooth */
    Serial.println("Initializing Bluetooth");
    bluetooth_init("Amperfy-ESP32");
    bluetooth_set_connection_callback(on_ble_connection);
    bluetooth_set_data_callback(on_ble_data);

    Serial.println("Setup complete!");
}

void loop()
{
    /* Handle Bluetooth events */
    bluetooth_update();

    /* Send library queries after connection (with delay for app to be ready) */
    if (g_should_query_library && bluetooth_is_connected()) {
        unsigned long elapsed = millis() - g_connection_time;
        if (elapsed >= QUERY_DELAY_MS) {
            g_should_query_library = false;
            Serial.print("[Main] Sending library queries after ");
            Serial.print(elapsed);
            Serial.println("ms delay");
            send_library_queries();
        }
    }

    delay(10);
}
