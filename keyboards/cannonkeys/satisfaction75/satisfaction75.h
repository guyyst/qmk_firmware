#pragma once

#include "quantum.h"

#ifdef KEYBOARD_cannonkeys_satisfaction75_prototype
#    include "prototype.h"
#else
#    include "rev1.h"
#endif

#include "via.h"  // only for EEPROM address
#define EEPROM_ENABLED_ENCODER_MODES (VIA_EEPROM_CUSTOM_CONFIG_ADDR)
#define EEPROM_CUSTOM_BACKLIGHT      (VIA_EEPROM_CUSTOM_CONFIG_ADDR + 1)
#define EEPROM_DEFAULT_OLED          (VIA_EEPROM_CUSTOM_CONFIG_ADDR + 2)
#define EEPROM_CUSTOM_ENCODER        (VIA_EEPROM_CUSTOM_CONFIG_ADDR + 3)

/* screen off after this many milliseconds */
#define ScreenOffInterval 60000 /* milliseconds */

typedef union {
    uint8_t raw;
    struct {
        bool    enable : 1;
        bool    breathing : 1;
        uint8_t level : 6;
    };
} backlight_config_t;

// Start these at the USER code range in VIA
enum my_keycodes { ENC_PRESS = 0x5F80, CLOCK_SET, OLED_TOGG };

enum s75_keyboard_value_id { id_encoder_modes = 0x80, id_oled_default_mode, id_encoder_custom, id_oled_mode };

enum encoder_modes {
    ENC_MODE_VOLUME,
    ENC_MODE_MEDIA,
    ENC_MODE_SCROLL,
    ENC_MODE_BRIGHTNESS,
    ENC_MODE_BACKLIGHT,
    ENC_MODE_CUSTOM0,
    ENC_MODE_CUSTOM1,
    ENC_MODE_CUSTOM2,
    _NUM_ENCODER_MODES,
    ENC_MODE_CLOCK_SET  // This shouldn't be included in the default modes, so we put it after NUM_ENCODER_MODES
};

enum custom_encoder_behavior { ENC_CUSTOM_CW = 0, ENC_CUSTOM_CCW, ENC_CUSTOM_PRESS };

enum oled_modes { OLED_DEFAULT, OLED_TIME, OLED_STATS, OLED_OFF, _NUM_OLED_MODES };

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

// Encoder Behavior
extern uint8_t encoder_value;
extern uint8_t encoder_mode;
extern uint8_t enabled_encoder_modes;

// RTC
extern RTCDateTime last_timespec;
extern uint16_t    last_minute;

// RTC Configuration
extern bool    clock_set_mode;
extern uint8_t time_config_idx;
extern int8_t  hour_config;
extern int16_t minute_config;
extern int8_t  year_config;
extern int8_t  month_config;
extern int8_t  day_config;
extern uint8_t previous_encoder_mode;

// Backlighting
extern backlight_config_t kb_backlight_config;
extern bool               kb_backlight_breathing;

void     pre_encoder_mode_change(void);
void     post_encoder_mode_change(void);
void     change_encoder_mode(bool negative);
uint16_t handle_encoder_clockwise(void);
uint16_t handle_encoder_ccw(void);
uint16_t handle_encoder_press(void);
uint16_t retrieve_custom_encoder_config(uint8_t encoder_idx, uint8_t behavior);
void     set_custom_encoder_config(uint8_t encoder_idx, uint8_t behavior, uint16_t new_code);

void update_time_config(int8_t increment);

__attribute__((weak)) void draw_ui(void);
void                       draw_default(void);
void                       draw_clock(void);
void                       draw_stats(void);

void backlight_init_ports(void);
void backlight_set(uint8_t level);
bool is_breathing(void);
void breathing_enable(void);
void breathing_disable(void);
void custom_config_load(void);
void backlight_config_save(void);

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

#    define KEYS_TO_COUNT    53
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
#    define KEYCODE_TO_CHAR(code)         (keycode_char_map[code])
#    define KEYCODE_TO_CHAR_SHIFTED(code) (keycode_char_map_shifted[code])
#    define IS_TRACKED_KEYCODE(code)      ((code) >= KC_A && (code) <= KC_SLASH && keycode_char_map[code - KC_A] != '\0')

#endif
