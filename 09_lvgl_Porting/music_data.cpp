/*
 * Music Data - Hardcoded Sample Data
 * Car Music Player Display
 */
#include "music_data.h"
#include <stdio.h>

// ============================================================================
// SONGS - The Beatles
// ============================================================================

static const Song song_beatles_1 = {1, "Come Together", "The Beatles", "Abbey Road", 259};
static const Song song_beatles_2 = {2, "Something", "The Beatles", "Abbey Road", 182};
static const Song song_beatles_3 = {3, "Here Comes The Sun", "The Beatles", "Abbey Road", 185};
static const Song song_beatles_4 = {4, "Octopus's Garden", "The Beatles", "Abbey Road", 171};
static const Song song_beatles_5 = {5, "Let It Be", "The Beatles", "Let It Be", 243};
static const Song song_beatles_6 = {6, "Across The Universe", "The Beatles", "Let It Be", 228};
static const Song song_beatles_7 = {7, "Get Back", "The Beatles", "Let It Be", 191};
static const Song song_beatles_8 = {8, "The Long And Winding Road", "The Beatles", "Let It Be", 222};

// ============================================================================
// SONGS - Pink Floyd
// ============================================================================

static const Song song_floyd_1 = {9, "Shine On You Crazy Diamond", "Pink Floyd", "Wish You Were Here", 810};
static const Song song_floyd_2 = {10, "Welcome To The Machine", "Pink Floyd", "Wish You Were Here", 450};
static const Song song_floyd_3 = {11, "Wish You Were Here", "Pink Floyd", "Wish You Were Here", 334};
static const Song song_floyd_4 = {12, "Have A Cigar", "Pink Floyd", "Wish You Were Here", 307};
static const Song song_floyd_5 = {13, "Comfortably Numb", "Pink Floyd", "The Wall", 382};
static const Song song_floyd_6 = {14, "Another Brick In The Wall", "Pink Floyd", "The Wall", 239};
static const Song song_floyd_7 = {15, "Hey You", "Pink Floyd", "The Wall", 280};
static const Song song_floyd_8 = {16, "Run Like Hell", "Pink Floyd", "The Wall", 254};
static const Song song_floyd_9 = {17, "Mother", "Pink Floyd", "The Wall", 334};

// ============================================================================
// SONGS - Led Zeppelin
// ============================================================================

static const Song song_zep_1 = {18, "Stairway To Heaven", "Led Zeppelin", "Led Zeppelin IV", 482};
static const Song song_zep_2 = {19, "Black Dog", "Led Zeppelin", "Led Zeppelin IV", 296};
static const Song song_zep_3 = {20, "Rock And Roll", "Led Zeppelin", "Led Zeppelin IV", 220};
static const Song song_zep_4 = {21, "Going To California", "Led Zeppelin", "Led Zeppelin IV", 206};
static const Song song_zep_5 = {22, "Kashmir", "Led Zeppelin", "Physical Graffiti", 517};
static const Song song_zep_6 = {23, "Trampled Under Foot", "Led Zeppelin", "Physical Graffiti", 342};
static const Song song_zep_7 = {24, "Ten Years Gone", "Led Zeppelin", "Physical Graffiti", 392};
static const Song song_zep_8 = {25, "In The Light", "Led Zeppelin", "Physical Graffiti", 526};

// ============================================================================
// SONGS - Queen
// ============================================================================

static const Song song_queen_1 = {26, "Bohemian Rhapsody", "Queen", "A Night at the Opera", 355};
static const Song song_queen_2 = {27, "You're My Best Friend", "Queen", "A Night at the Opera", 170};
static const Song song_queen_3 = {28, "Love of My Life", "Queen", "A Night at the Opera", 219};
static const Song song_queen_4 = {29, "We Will Rock You", "Queen", "News of the World", 122};
static const Song song_queen_5 = {30, "We Are The Champions", "Queen", "News of the World", 179};
static const Song song_queen_6 = {31, "Spread Your Wings", "Queen", "News of the World", 275};

// ============================================================================
// SONGS - The Rolling Stones
// ============================================================================

static const Song song_stones_1 = {32, "Paint It Black", "The Rolling Stones", "Aftermath", 222};
static const Song song_stones_2 = {33, "Under My Thumb", "The Rolling Stones", "Aftermath", 220};
static const Song song_stones_3 = {34, "Sympathy for the Devil", "The Rolling Stones", "Beggars Banquet", 378};
static const Song song_stones_4 = {35, "Street Fighting Man", "The Rolling Stones", "Beggars Banquet", 195};

// ============================================================================
// SONGS - The Who
// ============================================================================

static const Song song_who_1 = {36, "Baba O'Riley", "The Who", "Who's Next", 300};
static const Song song_who_2 = {37, "Behind Blue Eyes", "The Who", "Who's Next", 222};
static const Song song_who_3 = {38, "Won't Get Fooled Again", "The Who", "Who's Next", 510};
static const Song song_who_4 = {39, "Pinball Wizard", "The Who", "Tommy", 303};
static const Song song_who_5 = {40, "I'm Free", "The Who", "Tommy", 161};

// ============================================================================
// SONGS - Fleetwood Mac
// ============================================================================

static const Song song_fleet_1 = {41, "The Chain", "Fleetwood Mac", "Rumours", 270};
static const Song song_fleet_2 = {42, "Dreams", "Fleetwood Mac", "Rumours", 257};
static const Song song_fleet_3 = {43, "Go Your Own Way", "Fleetwood Mac", "Rumours", 217};
static const Song song_fleet_4 = {44, "Landslide", "Fleetwood Mac", "Fleetwood Mac", 199};
static const Song song_fleet_5 = {45, "Rhiannon", "Fleetwood Mac", "Fleetwood Mac", 276};

// ============================================================================
// SONGS - Eagles
// ============================================================================

static const Song song_eagles_1 = {46, "Hotel California", "Eagles", "Hotel California", 391};
static const Song song_eagles_2 = {47, "New Kid in Town", "Eagles", "Hotel California", 305};
static const Song song_eagles_3 = {48, "Life in the Fast Lane", "Eagles", "Hotel California", 280};
static const Song song_eagles_4 = {49, "Take It Easy", "Eagles", "Eagles", 211};
static const Song song_eagles_5 = {50, "Peaceful Easy Feeling", "Eagles", "Eagles", 244};

// ============================================================================
// SONGS - AC/DC
// ============================================================================

static const Song song_acdc_1 = {51, "Back in Black", "AC/DC", "Back in Black", 255};
static const Song song_acdc_2 = {52, "You Shook Me All Night Long", "AC/DC", "Back in Black", 210};
static const Song song_acdc_3 = {53, "Hells Bells", "AC/DC", "Back in Black", 312};
static const Song song_acdc_4 = {54, "Highway to Hell", "AC/DC", "Highway to Hell", 208};
static const Song song_acdc_5 = {55, "Touch Too Much", "AC/DC", "Highway to Hell", 270};

// ============================================================================
// SONGS - Aerosmith
// ============================================================================

static const Song song_aero_1 = {56, "Dream On", "Aerosmith", "Aerosmith", 267};
static const Song song_aero_2 = {57, "Walk This Way", "Aerosmith", "Toys in the Attic", 210};
static const Song song_aero_3 = {58, "Sweet Emotion", "Aerosmith", "Toys in the Attic", 270};
static const Song song_aero_4 = {59, "Toys in the Attic", "Aerosmith", "Toys in the Attic", 200};

// ============================================================================
// ALBUMS
// ============================================================================

// Beatles Albums
static const Song* abbey_road_songs[] = {
    &song_beatles_1, &song_beatles_2, &song_beatles_3, &song_beatles_4
};
static const Album album_abbey_road = {1, "Abbey Road", "The Beatles", abbey_road_songs, 4};

static const Song* let_it_be_songs[] = {
    &song_beatles_5, &song_beatles_6, &song_beatles_7, &song_beatles_8
};
static const Album album_let_it_be = {2, "Let It Be", "The Beatles", let_it_be_songs, 4};

// Pink Floyd Albums
static const Song* wish_you_were_here_songs[] = {
    &song_floyd_1, &song_floyd_2, &song_floyd_3, &song_floyd_4
};
static const Album album_wish_you_were_here = {3, "Wish You Were Here", "Pink Floyd", wish_you_were_here_songs, 4};

static const Song* the_wall_songs[] = {
    &song_floyd_5, &song_floyd_6, &song_floyd_7, &song_floyd_8, &song_floyd_9
};
static const Album album_the_wall = {4, "The Wall", "Pink Floyd", the_wall_songs, 5};

// Led Zeppelin Albums
static const Song* led_zep_iv_songs[] = {
    &song_zep_1, &song_zep_2, &song_zep_3, &song_zep_4
};
static const Album album_led_zep_iv = {5, "Led Zeppelin IV", "Led Zeppelin", led_zep_iv_songs, 4};

static const Song* physical_graffiti_songs[] = {
    &song_zep_5, &song_zep_6, &song_zep_7, &song_zep_8
};
static const Album album_physical_graffiti = {6, "Physical Graffiti", "Led Zeppelin", physical_graffiti_songs, 4};

// Queen Albums
static const Song* night_opera_songs[] = {
    &song_queen_1, &song_queen_2, &song_queen_3
};
static const Album album_night_opera = {7, "A Night at the Opera", "Queen", night_opera_songs, 3};

static const Song* news_world_songs[] = {
    &song_queen_4, &song_queen_5, &song_queen_6
};
static const Album album_news_world = {8, "News of the World", "Queen", news_world_songs, 3};

// Rolling Stones Albums
static const Song* aftermath_songs[] = {
    &song_stones_1, &song_stones_2
};
static const Album album_aftermath = {9, "Aftermath", "The Rolling Stones", aftermath_songs, 2};

static const Song* beggars_songs[] = {
    &song_stones_3, &song_stones_4
};
static const Album album_beggars = {10, "Beggars Banquet", "The Rolling Stones", beggars_songs, 2};

// The Who Albums
static const Song* whos_next_songs[] = {
    &song_who_1, &song_who_2, &song_who_3
};
static const Album album_whos_next = {11, "Who's Next", "The Who", whos_next_songs, 3};

static const Song* tommy_songs[] = {
    &song_who_4, &song_who_5
};
static const Album album_tommy = {12, "Tommy", "The Who", tommy_songs, 2};

// Fleetwood Mac Albums
static const Song* rumours_songs[] = {
    &song_fleet_1, &song_fleet_2, &song_fleet_3
};
static const Album album_rumours = {13, "Rumours", "Fleetwood Mac", rumours_songs, 3};

static const Song* fleetwood_songs[] = {
    &song_fleet_4, &song_fleet_5
};
static const Album album_fleetwood = {14, "Fleetwood Mac", "Fleetwood Mac", fleetwood_songs, 2};

// Eagles Albums
static const Song* hotel_cal_songs[] = {
    &song_eagles_1, &song_eagles_2, &song_eagles_3
};
static const Album album_hotel_cal = {15, "Hotel California", "Eagles", hotel_cal_songs, 3};

static const Song* eagles_debut_songs[] = {
    &song_eagles_4, &song_eagles_5
};
static const Album album_eagles_debut = {16, "Eagles", "Eagles", eagles_debut_songs, 2};

// AC/DC Albums
static const Song* back_black_songs[] = {
    &song_acdc_1, &song_acdc_2, &song_acdc_3
};
static const Album album_back_black = {17, "Back in Black", "AC/DC", back_black_songs, 3};

static const Song* highway_hell_songs[] = {
    &song_acdc_4, &song_acdc_5
};
static const Album album_highway_hell = {18, "Highway to Hell", "AC/DC", highway_hell_songs, 2};

// Aerosmith Albums
static const Song* aerosmith_debut_songs[] = {
    &song_aero_1
};
static const Album album_aerosmith_debut = {19, "Aerosmith", "Aerosmith", aerosmith_debut_songs, 1};

static const Song* toys_attic_songs[] = {
    &song_aero_2, &song_aero_3, &song_aero_4
};
static const Album album_toys_attic = {20, "Toys in the Attic", "Aerosmith", toys_attic_songs, 3};

// ============================================================================
// ARTISTS
// ============================================================================

static const Album* beatles_albums[] = {&album_abbey_road, &album_let_it_be};
static const Artist artist_beatles = {1, "The Beatles", beatles_albums, 2};

static const Album* floyd_albums[] = {&album_wish_you_were_here, &album_the_wall};
static const Artist artist_floyd = {2, "Pink Floyd", floyd_albums, 2};

static const Album* zeppelin_albums[] = {&album_led_zep_iv, &album_physical_graffiti};
static const Artist artist_zeppelin = {3, "Led Zeppelin", zeppelin_albums, 2};

static const Album* queen_albums[] = {&album_night_opera, &album_news_world};
static const Artist artist_queen = {4, "Queen", queen_albums, 2};

static const Album* stones_albums[] = {&album_aftermath, &album_beggars};
static const Artist artist_stones = {5, "The Rolling Stones", stones_albums, 2};

static const Album* who_albums[] = {&album_whos_next, &album_tommy};
static const Artist artist_who = {6, "The Who", who_albums, 2};

static const Album* fleetwood_albums[] = {&album_rumours, &album_fleetwood};
static const Artist artist_fleetwood = {7, "Fleetwood Mac", fleetwood_albums, 2};

static const Album* eagles_albums[] = {&album_hotel_cal, &album_eagles_debut};
static const Artist artist_eagles = {8, "Eagles", eagles_albums, 2};

static const Album* acdc_albums[] = {&album_back_black, &album_highway_hell};
static const Artist artist_acdc = {9, "AC/DC", acdc_albums, 2};

static const Album* aerosmith_albums[] = {&album_aerosmith_debut, &album_toys_attic};
static const Artist artist_aerosmith = {10, "Aerosmith", aerosmith_albums, 2};

// ============================================================================
// PLAYLISTS
// ============================================================================

// Classic Rock Favorites
static const Song* playlist_classic_rock_songs[] = {
    &song_beatles_1, &song_floyd_5, &song_zep_1, &song_beatles_5, &song_floyd_6,
    &song_zep_2, &song_beatles_3, &song_queen_1, &song_stones_1
};
static const Playlist playlist_classic_rock = {1, "Classic Rock Favorites", playlist_classic_rock_songs, 9};

// Chill Vibes
static const Song* playlist_chill_songs[] = {
    &song_beatles_3, &song_floyd_3, &song_zep_4, &song_beatles_2, &song_floyd_1,
    &song_zep_7, &song_fleet_2, &song_eagles_5
};
static const Playlist playlist_chill = {2, "Chill Vibes", playlist_chill_songs, 8};

// Road Trip Mix
static const Song* playlist_road_trip_songs[] = {
    &song_zep_2, &song_floyd_8, &song_beatles_7, &song_zep_5, &song_floyd_6,
    &song_beatles_1, &song_zep_3, &song_floyd_4, &song_acdc_1, &song_acdc_4
};
static const Playlist playlist_road_trip = {3, "Road Trip Mix", playlist_road_trip_songs, 10};

// 70s Anthems
static const Song* playlist_70s_songs[] = {
    &song_queen_1, &song_queen_4, &song_queen_5, &song_zep_1, &song_who_1,
    &song_eagles_1, &song_fleet_1
};
static const Playlist playlist_70s = {4, "70s Anthems", playlist_70s_songs, 7};

// Guitar Heroes
static const Song* playlist_guitar_songs[] = {
    &song_zep_1, &song_floyd_5, &song_acdc_1, &song_aero_2, &song_who_3,
    &song_eagles_1, &song_stones_3
};
static const Playlist playlist_guitar = {5, "Guitar Heroes", playlist_guitar_songs, 7};

// Power Ballads
static const Song* playlist_ballads_songs[] = {
    &song_queen_3, &song_zep_4, &song_aero_1, &song_fleet_4, &song_beatles_5,
    &song_who_2
};
static const Playlist playlist_ballads = {6, "Power Ballads", playlist_ballads_songs, 6};

// Arena Rock
static const Song* playlist_arena_songs[] = {
    &song_queen_4, &song_queen_5, &song_acdc_1, &song_who_3, &song_zep_2,
    &song_aero_2, &song_stones_1
};
static const Playlist playlist_arena = {7, "Arena Rock", playlist_arena_songs, 7};

// British Invasion
static const Song* playlist_british_songs[] = {
    &song_beatles_1, &song_beatles_3, &song_stones_1, &song_stones_3, &song_who_1,
    &song_floyd_5, &song_zep_1, &song_queen_1
};
static const Playlist playlist_british = {8, "British Invasion", playlist_british_songs, 8};

// Driving Songs
static const Song* playlist_driving_songs[] = {
    &song_acdc_4, &song_eagles_3, &song_zep_2, &song_stones_4, &song_who_3,
    &song_fleet_3, &song_aero_2
};
static const Playlist playlist_driving = {9, "Driving Songs", playlist_driving_songs, 7};

// Late Night
static const Song* playlist_night_songs[] = {
    &song_floyd_1, &song_floyd_3, &song_beatles_6, &song_fleet_2, &song_eagles_1,
    &song_queen_3
};
static const Playlist playlist_night = {10, "Late Night", playlist_night_songs, 6};

// ============================================================================
// GLOBAL ARRAYS
// ============================================================================

const Song* const ALL_SONGS[] = {
    &song_beatles_1, &song_beatles_2, &song_beatles_3, &song_beatles_4,
    &song_beatles_5, &song_beatles_6, &song_beatles_7, &song_beatles_8,
    &song_floyd_1, &song_floyd_2, &song_floyd_3, &song_floyd_4,
    &song_floyd_5, &song_floyd_6, &song_floyd_7, &song_floyd_8, &song_floyd_9,
    &song_zep_1, &song_zep_2, &song_zep_3, &song_zep_4,
    &song_zep_5, &song_zep_6, &song_zep_7, &song_zep_8,
    &song_queen_1, &song_queen_2, &song_queen_3, &song_queen_4, &song_queen_5, &song_queen_6,
    &song_stones_1, &song_stones_2, &song_stones_3, &song_stones_4,
    &song_who_1, &song_who_2, &song_who_3, &song_who_4, &song_who_5,
    &song_fleet_1, &song_fleet_2, &song_fleet_3, &song_fleet_4, &song_fleet_5,
    &song_eagles_1, &song_eagles_2, &song_eagles_3, &song_eagles_4, &song_eagles_5,
    &song_acdc_1, &song_acdc_2, &song_acdc_3, &song_acdc_4, &song_acdc_5,
    &song_aero_1, &song_aero_2, &song_aero_3, &song_aero_4
};
const uint8_t ALL_SONGS_COUNT = 59;

const Album* const ALL_ALBUMS[] = {
    &album_abbey_road, &album_let_it_be,
    &album_wish_you_were_here, &album_the_wall,
    &album_led_zep_iv, &album_physical_graffiti,
    &album_night_opera, &album_news_world,
    &album_aftermath, &album_beggars,
    &album_whos_next, &album_tommy,
    &album_rumours, &album_fleetwood,
    &album_hotel_cal, &album_eagles_debut,
    &album_back_black, &album_highway_hell,
    &album_aerosmith_debut, &album_toys_attic
};
const uint8_t ALL_ALBUMS_COUNT = 20;

const Artist* const ALL_ARTISTS[] = {
    &artist_beatles, &artist_floyd, &artist_zeppelin,
    &artist_queen, &artist_stones, &artist_who,
    &artist_fleetwood, &artist_eagles, &artist_acdc, &artist_aerosmith
};
const uint8_t ALL_ARTISTS_COUNT = 10;

const Playlist* const ALL_PLAYLISTS[] = {
    &playlist_classic_rock, &playlist_chill, &playlist_road_trip,
    &playlist_70s, &playlist_guitar, &playlist_ballads,
    &playlist_arena, &playlist_british, &playlist_driving, &playlist_night
};
const uint8_t ALL_PLAYLISTS_COUNT = 10;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

const Song* get_song_by_id(uint16_t id) {
    for (uint8_t i = 0; i < ALL_SONGS_COUNT; i++) {
        if (ALL_SONGS[i]->id == id) {
            return ALL_SONGS[i];
        }
    }
    return nullptr;
}

const Album* get_album_by_id(uint16_t id) {
    for (uint8_t i = 0; i < ALL_ALBUMS_COUNT; i++) {
        if (ALL_ALBUMS[i]->id == id) {
            return ALL_ALBUMS[i];
        }
    }
    return nullptr;
}

const Artist* get_artist_by_id(uint16_t id) {
    for (uint8_t i = 0; i < ALL_ARTISTS_COUNT; i++) {
        if (ALL_ARTISTS[i]->id == id) {
            return ALL_ARTISTS[i];
        }
    }
    return nullptr;
}

const Playlist* get_playlist_by_id(uint16_t id) {
    for (uint8_t i = 0; i < ALL_PLAYLISTS_COUNT; i++) {
        if (ALL_PLAYLISTS[i]->id == id) {
            return ALL_PLAYLISTS[i];
        }
    }
    return nullptr;
}

const char* format_duration(uint16_t seconds) {
    static char buffer[16];
    uint16_t minutes = seconds / 60;
    uint16_t secs = seconds % 60;
    snprintf(buffer, sizeof(buffer), "%d:%02d", minutes, secs);
    return buffer;
}
