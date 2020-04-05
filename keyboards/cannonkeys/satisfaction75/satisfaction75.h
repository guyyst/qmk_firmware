#pragma once

#include "quantum.h"
#include "rev1.h"

/* screen off after this many milliseconds */
#define ScreenOffInterval 60000 /* milliseconds */

// Start these at the USER code range in VIA
enum my_keycodes { ENC_PRESS = 0x5F80, OLED_TOGG };

enum oled_modes { OLED_DEFAULT, OLED_STATS, OLED_SNAKE, OLED_OFF, _NUM_OLED_MODES, OLED_DELETE };

// Keyboard Information
extern volatile uint8_t led_numlock;
extern volatile uint8_t led_capslock;
extern volatile uint8_t led_scrolllock;
extern uint8_t          layer;

// OLED Behavior
extern uint16_t last_flush;
extern bool     queue_for_send;
extern uint8_t  oled_mode;
extern bool     oled_sleeping;

__attribute__((weak)) void draw_ui(void);
void                       draw_default(void);
void                       draw_encoder(int8_t startX, int8_t startY, bool show_legend);
void                       draw_stats(void);
void                       draw_delete(void);

#ifdef ENABLE_SNAKE_MODE
void draw_snake(void);
void init_game(void);
void change_game_zoom(void);
enum SnakeDirections { NONE = 0, UP = 1, DOWN = -1, LEFT = 2, RIGHT = -2};
extern int8_t desired_snake_direction;
#endif

// Timer for spamming delete
extern uint16_t delete_timer;
#define DELETE_SPAM_TIME 200
#define DELETE_FLASH_INTERVAL 2

// Key stat and history tracking.
#ifdef ENABLE_STAT_TRACKING

/**
 * The tracker works by relying on the proximity of the tracked keycode values as defined in keycode.h.
 * From KC_A (4) to KC_SLASH (56) we span 53 keys that include all the symbols needing to be tracked,
 * as well as a few extra ones we can ignore.
 *
 * To prevent calculation of percentages in matrix_scan_kb(), both the total and the percentage of each key is stored in two arrays.
 * On each key press the keys with the top 4 percentages are saved in an extra array.
 *
 * Conversion to the actual character for display purposes is made easy by mapping each keycode (0 - 53) to a character.
 * The leaderboard will only ever show the unshifted keys, but the 9-key history stores chars directly and needs the capitalized version,
 * which is the reason fot having two arrays: keycode_char_map and keycode_char_map_shifted.
 *
 * Should you want the keycodes to be mapped to different characters for other languages simply adjust the two arrays in satisfaction_oled.c.
 * '\0' characters will remain uncounted. Same goes for the spacebar, which only shows up in the history but does not get used in the stats.
 */

#    define KEYS_TO_COUNT 53
#    define LEADERBOARD_SIZE 4
#    define KEY_HISTORY_SIZE 9
extern uint32_t key_total_counter;
extern uint32_t key_counters[KEYS_TO_COUNT];
extern float    key_percentages[KEYS_TO_COUNT];
extern int8_t   key_leaderboard[LEADERBOARD_SIZE];
extern char     key_history[KEY_HISTORY_SIZE];
extern int8_t   key_history_head;

extern char keycode_char_map[KEYS_TO_COUNT];
extern char keycode_char_map_shifted[KEYS_TO_COUNT];
#    define KEYCODE_TO_CHAR(code) (keycode_char_map[code])
#    define KEYCODE_TO_CHAR_SHIFTED(code) (keycode_char_map_shifted[code])
#    define IS_TRACKED_KEYCODE(code) ((code) >= KC_A && (code) <= KC_SLASH && keycode_char_map[code - KC_A] != '\0')

#endif
