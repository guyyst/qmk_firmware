#include "satisfaction75.h"
#include "print.h"
#include "debug.h"
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "micro_oled.h"
#include "qwiic.h"

#include "timer.h"

#include "raw_hid.h"
#include "dynamic_keymap.h"
#include "tmk_core/common/eeprom.h"
#include "version.h"  // for QMK_BUILDDATE used in EEPROM magic

/* Artificial delay added to get media keys to work in the encoder*/
#define MEDIA_KEY_DELAY 10

/* In milliseconds, how close together next_track and prev_track need to be pressed to send pause_track instead */
#define MEDIA_KEY_PAUSE_TIMEOUT 140
uint16_t media_next_track_timer = 0;
uint16_t media_prev_track_timer = 0;

bool caps_used_as_fn = false;

uint16_t last_flush;

volatile uint8_t led_numlock    = false;
volatile uint8_t led_capslock   = false;
volatile uint8_t led_scrolllock = false;

uint8_t layer;

bool    queue_for_send = false;
uint8_t oled_mode      = OLED_DEFAULT;
bool    oled_sleeping  = false;

uint16_t delete_timer = 0;

void read_host_led_state(void) {
    uint8_t leds = host_keyboard_leds();
    if (leds & (1 << USB_LED_NUM_LOCK)) {
        if (led_numlock == false) {
            led_numlock = true;
        }
    } else {
        if (led_numlock == true) {
            led_numlock = false;
        }
    }
    if (leds & (1 << USB_LED_CAPS_LOCK)) {
        if (led_capslock == false) {
            led_capslock = true;
        }
    } else {
        if (led_capslock == true) {
            led_capslock = false;
        }
    }
    if (leds & (1 << USB_LED_SCROLL_LOCK)) {
        if (led_scrolllock == false) {
            led_scrolllock = true;
        }
    } else {
        if (led_scrolllock == true) {
            led_scrolllock = false;
        }
    }
}

uint32_t layer_state_set_kb(uint32_t state) {
    state          = layer_state_set_user(state);
    layer          = biton32(state);
    queue_for_send = true;
    return state;
}

#ifdef ENABLE_STAT_TRACKING

uint32_t key_total_counter;
uint32_t key_counters[KEYS_TO_COUNT];
float    key_percentages[KEYS_TO_COUNT];
int8_t   key_leaderboard[LEADERBOARD_SIZE];
char     key_history[KEY_HISTORY_SIZE];
int8_t   key_history_head;

/**
 * Loop over the current leaderboard, potentially insert the provided key and adjust the leadboard.
 */
static void insert_leaderboard(uint8_t key_idx) {
    for (uint8_t k = 0; k < LEADERBOARD_SIZE; k++) {
        // Leave if the given index is already in the leaderboard.
        if (key_idx == key_leaderboard[k]) break;

        // If a slot is empty and the given key is above 0% just override and leave.
        if (key_leaderboard[k] == -1) {
            if (key_percentages[key_idx] > 0) {
                key_leaderboard[k] = key_idx;
            }

            return;
        }

        if (key_percentages[key_idx] > key_percentages[key_leaderboard[k]]) {
            // Indicates what the lowest leaderboard spot is we need to shift the higher ones down to.
            // E.g. if a new key is inserted at 0 we need to shift all the way down to 3.
            // Whereas inserting the key previously held in 2 into spot 0 would only require us to shift down to spot 2.
            uint8_t shift_end;

            for (shift_end = k + 1; shift_end < LEADERBOARD_SIZE; shift_end++) {
                if (key_leaderboard[shift_end] == key_idx) {
                    break;
                }
            }

            // Shift down keys starting at spot shift_end and moving up to spot k.
            for (uint8_t i = shift_end; i > k; i--) {
                key_leaderboard[i] = key_leaderboard[i - 1];
            }

            // Finally after shifting everything around insert the new key at k.
            key_leaderboard[k] = key_idx;

            return;
        }
    }
}

static void reset_key_stats(void) {
    key_total_counter = 0;
    key_history_head  = 0;

    memset(key_counters, 0, sizeof(key_counters));
    memset(key_percentages, 0, sizeof(key_percentages));
    memset(key_leaderboard, -1, sizeof(key_leaderboard));
    memset(key_history, -1, sizeof(key_history));
}

static inline void update_key_stats(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        if (IS_TRACKED_KEYCODE(keycode)) {
            keycode -= KC_A;  // Shift everything down to have A start at '0' and '/' end at 52

            key_total_counter++;
            key_counters[keycode]++;

            for (uint8_t i = 0; i < KEYS_TO_COUNT; i++) {
                key_percentages[i] = (float)key_counters[i] / key_total_counter;

                insert_leaderboard(i);
            }

            // Check if the key was shifted and store its char in the key history.
            bool key_shifted              = (led_capslock == true) || (get_mods() & MOD_MASK_SHIFT);
            key_history_head              = (key_history_head + 1) % 9;
            key_history[key_history_head] = key_shifted ? KEYCODE_TO_CHAR_SHIFTED(keycode) : KEYCODE_TO_CHAR(keycode);
        } else if (keycode == KC_SPACE) {
            // Space is not a tracked keycode but should still appear in the history for readability.
            key_history_head              = (key_history_head + 1) % 9;
            key_history[key_history_head] = ' ';
        }
    }
}

#endif

void toggle_delete_spam(void) {
    if (delete_timer == 0) {
        delete_timer = timer_read();
        oled_mode    = OLED_DELETE;
    } else {
        delete_timer = 0;
        oled_mode    = OLED_DEFAULT;
    }
}

void tap_media_key(uint16_t keycode) {
    uint16_t held_keycode_timer = timer_read();
    register_code16(keycode);
    while (timer_elapsed(held_keycode_timer) < MEDIA_KEY_DELAY) {
        /* no-op */
    }
    unregister_code16(keycode);
}

bool caps_to_shift(uint16_t keycode, keyrecord_t *record) {
    if ((keycode >= KC_1 && keycode <= KC_0) || (keycode >= KC_MINUS && keycode <= KC_SLASH)) {
        read_host_led_state();

        if (led_capslock == true) {
            if (record->event.pressed) {
                register_code16(LSFT(keycode));
            } else {
                unregister_code16(LSFT(keycode));
            }

            return false;
        }
    }

    return true;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
#ifdef ENABLE_STAT_TRACKING
    update_key_stats(keycode, record);
#endif

    
#ifdef ENABLE_SNAKE_MODE
    if (oled_mode == OLED_SNAKE && record->event.pressed) {
        switch (keycode) {
            case KC_UP:
                desired_snake_direction = UP;
                break;
            case KC_DOWN:
                desired_snake_direction = DOWN;
                break;
            case KC_LEFT:
                desired_snake_direction = LEFT;
                break;
            case KC_RIGHT:
                desired_snake_direction = RIGHT;
                break;
        }
    }
#endif


    if (record->event.pressed) {
        caps_used_as_fn = true;
    }

    queue_for_send = true;
    switch (keycode) {
        case KC_LEFT:
        case KC_RIGHT: {
            if (layer == 1) {
                
                register_code(keycode == KC_LEFT ? KC_HOME : KC_END);
                unregister_code(keycode == KC_LEFT ? KC_HOME : KC_END);

                return false;
            }

            break;
        }
        case KC_CAPS:
            if (record->event.pressed) {
                layer_on(1);
                caps_used_as_fn = false;
            } else {
                layer_off(1);
                if (!caps_used_as_fn) {
                    register_code(KC_CAPS);
                    unregister_code(KC_CAPS);
                }
            }
            return false;
        case KC_ESC:
            if (record->event.pressed) {
                // Make Win+Esc send Alt+F4
                if (get_mods() & MOD_MASK_GUI) {
                    register_code(KC_LALT);
                    del_mods(MOD_MASK_GUI);
                    register_code(KC_F4);
                    unregister_code(KC_F4);
                    unregister_code(KC_LALT);
                    return false;
                }

#ifdef ENABLE_STAT_TRACKING
                // Make Fn+Esc reset key history and stats
                if (layer == 1) {
                    reset_key_stats();
                    return false;
                }
#endif
            }
            break;
        case KC_MEDIA_NEXT_TRACK:
            if (record->event.pressed) {
                media_next_track_timer = timer_read();
            } else if (media_next_track_timer != 0) {
                media_next_track_timer = 0;
                tap_media_key(KC_MEDIA_NEXT_TRACK);
            }
            return false;
        case KC_MEDIA_PREV_TRACK:
            if (record->event.pressed) {
                media_prev_track_timer = timer_read();
            } else if (media_prev_track_timer != 0) {
                media_prev_track_timer = 0;
                tap_media_key(KC_MEDIA_PREV_TRACK);
            }
            return false;
        case KC_DELETE: {
            if (layer == 1 && record->event.pressed) {
                toggle_delete_spam();
                return false;
            }
        } break;
        case OLED_TOGG:
            if (record->event.pressed && oled_mode < _NUM_OLED_MODES) {
                oled_mode = (oled_mode + 1) % _NUM_OLED_MODES;
                draw_ui();
            }
            return false;
        case ENC_PRESS:
            if (record->event.pressed) {
                tap_media_key(KC_MUTE);
            } else {
                // Do something else when release
            }
            return false;
        default:
            if (!caps_to_shift(keycode, record)) {
                return false;
            }
            break;
    }

    return process_record_user(keycode, record);
}

void encoder_update_kb(uint8_t index, bool clockwise) {
    queue_for_send = true;
    if (index == 0) {
        if (get_mods() & MOD_MASK_GUI) {
            // Make Win+Encoder send Win+Plus/Minus
            if (clockwise) {
                register_code(KC_KP_PLUS);
                unregister_code(KC_KP_PLUS);
            } else {
                register_code(KC_KP_MINUS);
                unregister_code(KC_KP_MINUS);
            }
        } else if (get_mods() & MOD_MASK_CTRL) {
            if (clockwise) {
                register_code(KC_Y);
                unregister_code(KC_Y);
            } else {
                register_code(KC_Z);
                unregister_code(KC_Z);
            }
        } else if (layer == 0) {
            if (clockwise) {
                tap_media_key(KC_VOLU);
            } else {
                tap_media_key(KC_VOLD);
            }
        }
    }
}

void matrix_init_kb(void) {
#ifdef ENABLE_STAT_TRACKING
    reset_key_stats();
#endif

    queue_for_send = true;
    matrix_init_user();
}

void matrix_scan_kb(void) {
    if (delete_timer != 0) {
        if (timer_elapsed(delete_timer) > DELETE_SPAM_TIME) {
            register_code(KC_DELETE);
            unregister_code(KC_DELETE);

            delete_timer = timer_read();
            draw_ui();
        }

        return;
    }
    
    #ifdef ENABLE_SNAKE_MODE
    if (oled_mode == OLED_SNAKE) {
        draw_ui();
        return;
    }
    #endif

    if (queue_for_send) {
        oled_sleeping = false;
        read_host_led_state();
        draw_ui();
        queue_for_send = false;
    }

    if (timer_elapsed(last_flush) > ScreenOffInterval && !oled_sleeping) {
        send_command(DISPLAYOFF); /* 0xAE */
        oled_sleeping = true;
    }

    // If both next_track and prev_track have been pressed in the last milliseconds, send pause_track instead.
    if (media_next_track_timer != 0 && media_prev_track_timer != 0 && timer_elapsed(media_next_track_timer) < MEDIA_KEY_PAUSE_TIMEOUT && timer_elapsed(media_prev_track_timer) < MEDIA_KEY_PAUSE_TIMEOUT) {
        tap_media_key(KC_MEDIA_PLAY_PAUSE);
        media_next_track_timer = 0;
        media_prev_track_timer = 0;
    }
}
