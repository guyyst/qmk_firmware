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

uint16_t last_flush;

volatile uint8_t led_numlock    = false;
volatile uint8_t led_capslock   = false;
volatile uint8_t led_scrolllock = false;

uint8_t layer;

bool    queue_for_send = false;
uint8_t oled_mode      = OLED_DEFAULT;
bool    oled_sleeping  = false;

uint16_t delete_timer = 0;

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

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
#ifdef ENABLE_STAT_TRACKING
    update_key_stats(keycode, record);
#endif

    queue_for_send = true;
    switch (keycode) {
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
                uint16_t held_keycode_timer = timer_read();
                register_code16(KC_MUTE);
                while (timer_elapsed(held_keycode_timer) < MEDIA_KEY_DELAY) { /* no-op */
                }
                unregister_code16(KC_MUTE);
            } else {
                // Do something else when release
            }
            return false;
#ifdef ENABLE_STAT_TRACKING
        case KC_ESC:
            if (record->event.pressed) {
                if (layer == 1) {
                    reset_key_stats();
                    return false;
                }
            }
            break;
#endif
        default:
            break;
    }

    return process_record_user(keycode, record);
}

void encoder_update_kb(uint8_t index, bool clockwise) {
    queue_for_send = true;
    if (index == 0) {
        if (layer == 0) {
            uint16_t mapped_code = 0;
            if (clockwise) {
                mapped_code = KC_VOLU;
            } else {
                mapped_code = KC_VOLD;
            }
            uint16_t held_keycode_timer = timer_read();
            if (mapped_code != 0) {
                register_code16(mapped_code);
                while (timer_elapsed(held_keycode_timer) < MEDIA_KEY_DELAY) { /* no-op */
                }
                unregister_code16(mapped_code);
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

    if (queue_for_send && oled_mode != OLED_OFF) {
        oled_sleeping = false;
        read_host_led_state();
        draw_ui();
        queue_for_send = false;
    }
    if (timer_elapsed(last_flush) > ScreenOffInterval && !oled_sleeping) {
        send_command(DISPLAYOFF); /* 0xAE */
        oled_sleeping = true;
    }
}
