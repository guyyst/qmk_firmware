#include "satisfaction75.h"
#include "micro_oled.h"

__attribute__((weak)) void draw_ui() {
    clear_buffer();
    last_flush = timer_read();
    send_command(DISPLAYON);
    switch (oled_mode) {
        default:
        case OLED_DEFAULT:
            draw_default();
            break;
#ifdef ENABLE_STAT_TRACKING
        case OLED_STATS:
            draw_stats();
            break;
#endif
        case OLED_OFF:
            send_command(DISPLAYOFF);
            break;
    }
}

void draw_layer_section(int8_t startX, int8_t startY, bool show_legend) {
    if (show_legend) {
        draw_string(startX + 1, startY + 2, "LAYER", PIXEL_ON, NORM, 0);
    } else {
        startX -= 32;
    }
    draw_rect_filled_soft(startX + 32, startY + 1, 9, 9, PIXEL_ON, NORM);
    draw_char(startX + 34, startY + 2, layer + 0x30, PIXEL_ON, XOR, 0);
}

void draw_default() {
    uint8_t mods = get_mods();

    /* Layer indicator is 41 x 10 pixels */
    draw_layer_section(0, 0, true);

#define ENCODER_INDICATOR_X 45
#define ENCODER_INDICATOR_Y 0
    draw_encoder(ENCODER_INDICATOR_X, ENCODER_INDICATOR_Y, true);
/* Matrix display is 19 x 9 pixels */
#define MATRIX_DISPLAY_X 0
#define MATRIX_DISPLAY_Y 18

    for (uint8_t x = 0; x < MATRIX_ROWS; x++) {
        for (uint8_t y = 0; y < MATRIX_COLS; y++) {
            draw_pixel(MATRIX_DISPLAY_X + y + 2, MATRIX_DISPLAY_Y + x + 2, (matrix_get_row(x) & (1 << y)) > 0, NORM);
        }
    }
    draw_rect_soft(MATRIX_DISPLAY_X, MATRIX_DISPLAY_Y, 19, 9, PIXEL_ON, NORM);
    /* hadron oled location on thumbnail */
    draw_rect_filled_soft(MATRIX_DISPLAY_X + 14, MATRIX_DISPLAY_Y + 2, 3, 1, PIXEL_ON, NORM);

/* Mod display is 41 x 16 pixels */
#define MOD_DISPLAY_X 30
#define MOD_DISPLAY_Y 18

    if (mods & MOD_LSFT) {
        draw_rect_filled_soft(MOD_DISPLAY_X + 0, MOD_DISPLAY_Y, 5 + (1 * 6), 11, PIXEL_ON, NORM);
        draw_string(MOD_DISPLAY_X + 3, MOD_DISPLAY_Y + 2, "S", PIXEL_OFF, NORM, 0);
    } else {
        draw_string(MOD_DISPLAY_X + 3, MOD_DISPLAY_Y + 2, "S", PIXEL_ON, NORM, 0);
    }
    if (mods & MOD_LCTL) {
        draw_rect_filled_soft(MOD_DISPLAY_X + 10, MOD_DISPLAY_Y, 5 + (1 * 6), 11, PIXEL_ON, NORM);
        draw_string(MOD_DISPLAY_X + 13, MOD_DISPLAY_Y + 2, "C", PIXEL_OFF, NORM, 0);
    } else {
        draw_string(MOD_DISPLAY_X + 13, MOD_DISPLAY_Y + 2, "C", PIXEL_ON, NORM, 0);
    }
    if (mods & MOD_LALT) {
        draw_rect_filled_soft(MOD_DISPLAY_X + 20, MOD_DISPLAY_Y, 5 + (1 * 6), 11, PIXEL_ON, NORM);
        draw_string(MOD_DISPLAY_X + 23, MOD_DISPLAY_Y + 2, "A", PIXEL_OFF, NORM, 0);
    } else {
        draw_string(MOD_DISPLAY_X + 23, MOD_DISPLAY_Y + 2, "A", PIXEL_ON, NORM, 0);
    }
    if (mods & MOD_LGUI) {
        draw_rect_filled_soft(MOD_DISPLAY_X + 30, MOD_DISPLAY_Y, 5 + (1 * 6), 11, PIXEL_ON, NORM);
        draw_string(MOD_DISPLAY_X + 33, MOD_DISPLAY_Y + 2, "G", PIXEL_OFF, NORM, 0);
    } else {
        draw_string(MOD_DISPLAY_X + 33, MOD_DISPLAY_Y + 2, "G", PIXEL_ON, NORM, 0);
    }

/* Lock display is 23 x 21 */
#define LOCK_DISPLAY_X 100
#define LOCK_DISPLAY_Y 0

    if (led_capslock == true) {
        draw_rect_filled_soft(LOCK_DISPLAY_X + 0, LOCK_DISPLAY_Y, 5 + (3 * 6), 9, PIXEL_ON, NORM);
        draw_string(LOCK_DISPLAY_X + 3, LOCK_DISPLAY_Y + 1, "CAP", PIXEL_OFF, NORM, 0);
    } else if (led_capslock == false) {
        draw_string(LOCK_DISPLAY_X + 3, LOCK_DISPLAY_Y + 1, "CAP", PIXEL_ON, NORM, 0);
    }

    if (led_scrolllock == true) {
        draw_rect_filled_soft(LOCK_DISPLAY_X + 0, LOCK_DISPLAY_Y + 11, 5 + (3 * 6), 9, PIXEL_ON, NORM);
        draw_string(LOCK_DISPLAY_X + 3, LOCK_DISPLAY_Y + 11 + 1, "SCR", PIXEL_OFF, NORM, 0);
    } else if (led_scrolllock == false) {
        draw_string(LOCK_DISPLAY_X + 3, LOCK_DISPLAY_Y + 11 + 1, "SCR", PIXEL_ON, NORM, 0);
    }

    send_buffer();
}

void draw_encoder(int8_t startX, int8_t startY, bool show_legend) {
    if (show_legend) {
        draw_string(startX + 1, startY + 2, "ENC", PIXEL_ON, NORM, 0);
    } else {
        startX -= 22;
    }
    draw_rect_filled_soft(startX + 22, startY + 1, 3 + (3 * 6), 9, PIXEL_ON, NORM);
    char *mode_string = "";
    mode_string       = "VOL";
    draw_string(startX + 24, startY + 2, mode_string, PIXEL_ON, XOR, 0);
}

#ifdef ENABLE_STAT_TRACKING

char keycode_char_map[53] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0', '\0', '\0', '\0', '\0', '-', '=', '[', ']', '\\', '\0', ';', '\'', '`', ',', '.', '/'};

char keycode_char_map_shifted[53] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '\0', '\0', '\0', '\0', '\0', '_', '+', '{', '}', '|', '\0', ':', '|', '~', '<', '>', '?'};

void draw_stats() {
    for (uint8_t spot = 0; spot < LEADERBOARD_SIZE; spot++) {
        // Only draw leaderboard spot if it's not empty
        if (key_leaderboard[spot] != -1) {
            // If the keycode is less than 26 (i.e. a letter) we want the shifted version of the char for better readability.
            char key_str = key_leaderboard[spot] < 26 ? KEYCODE_TO_CHAR_SHIFTED(key_leaderboard[spot]) : KEYCODE_TO_CHAR(key_leaderboard[spot]);

            uint8_t percentage  = key_percentages[key_leaderboard[spot]] * 100;
            char    perc_str[4] = "";
            sprintf(perc_str, "%2u%%", percentage);

#    define X_OFFSET 4
#    define RIGHT_X_OFFSET 52
#    define BOTTOM_Y_OFFSET 17

            uint8_t x_pos = X_OFFSET + (spot % 2) * RIGHT_X_OFFSET;
            uint8_t y_pos = (spot > 1) * BOTTOM_Y_OFFSET;

            draw_rect_soft(x_pos, y_pos + (spot > 1), 11, 13, PIXEL_ON, NORM);
            draw_char(x_pos + 3, y_pos + 4 - (spot <= 1), key_str, PIXEL_ON, PIXEL_ON, 0);

            // Special handling for very first key press. 100% is too wide to print normally so this is kinda kerning it.
            if (percentage == 100) {
                draw_char(x_pos + 14, y_pos + 1, '1', PIXEL_ON, NORM, 1);
                draw_char(x_pos + 20, y_pos + 1, '0', PIXEL_ON, NORM, 1);
                draw_char(x_pos + 27, y_pos + 1, '0', PIXEL_ON, NORM, 1);
                draw_char(x_pos + 35, y_pos + 1, '%', PIXEL_ON, NORM, 1);
            } else {
                draw_string(x_pos + 15, y_pos + 1, perc_str, PIXEL_ON, NORM, 1);
            }
        }
    }

    // Draw the grid around the leaderboard
    draw_rect_filled(0, 15, 100, 1, PIXEL_ON, NORM);
    draw_rect_filled(50, 0, 1, 31, PIXEL_ON, NORM);
    draw_rect_filled(100, 0, 1, 31, PIXEL_ON, NORM);

    // Draw the key history to the right of the leaderboard
    for (int8_t i = 8; i >= 0; i--) {
        char key = key_history[(key_history_head + 1 + i) % 9];
        if ((int8_t)key != -1) {
            draw_char(105 + 9 * (i % 3), 1 + 10 * (i / 3), key, PIXEL_ON, PIXEL_ON, 0);
        }
    }

    send_buffer();
}

#endif
