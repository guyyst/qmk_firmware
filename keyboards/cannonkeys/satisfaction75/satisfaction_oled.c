#include "satisfaction75.h"
#include "micro_oled.h"
#include "time.h"

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
#ifdef ENABLE_SNAKE_MODE
        case OLED_SNAKE:
            draw_snake();
            break;
#endif
        case OLED_DELETE:
            draw_delete();
            break;
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

static uint8_t delete_flash_counter = 0;
void           draw_delete() {
    ++delete_flash_counter;

    if (delete_flash_counter <= DELETE_FLASH_INTERVAL) {
        draw_string(36, 12, "DELETE", PIXEL_ON, NORM, 1);
        send_buffer();
    } else if (delete_flash_counter <= DELETE_FLASH_INTERVAL * 2) {
        send_command(DISPLAYOFF);
    } else {
        delete_flash_counter = 0;
    }
}

#ifdef ENABLE_SNAKE_MODE

uint8_t zoom_index = 3;
#define GAME_ZOOM (1 << zoom_index)
#define GAME_WIDTH (128 / GAME_ZOOM)
#define GAME_HEIGHT (32 / GAME_ZOOM)

void change_game_zoom(bool increase) {
    if (increase && zoom_index < 4) {
        zoom_index++;
    } else if (!increase && zoom_index > 0) {
        zoom_index--;
    }
    
    init_game();
}
#define SNAKE_STEP_TIME_MIN 100
#define SNAKE_STEP_ZOOM_SCALE ( \
    zoom_index == 0 ? 0.4f : \
    zoom_index == 1 ? 0.6f : \
    zoom_index == 3 ? 1.4f : \
    zoom_index == 4 ? 1.7f : 1.0f)
#define SNAKE_STEP_LENGTH_ADDITION(score) ((score < 30 ? (30 - score) : 0) * 2.2f)
#define GET_SNAKE_STEP_TIME(score) ((uint16_t)((SNAKE_STEP_TIME_MIN + SNAKE_STEP_LENGTH_ADDITION(score)) * SNAKE_STEP_ZOOM_SCALE))

uint16_t snake_step_timer = 0;

typedef struct {
    uint8_t x;
    uint8_t y;
} GamePos;

GamePos food_pos;

GamePos *snake_links;
uint16_t num_snake_links = 0;

int8_t current_snake_direction = NONE;
int8_t desired_snake_direction = NONE;

bool game_lost = false;

bool is_pos_snake(GamePos pos) {
    for (size_t i = 0; i < num_snake_links; i++) {
        if (snake_links[i].x == pos.x && snake_links[i].y == pos.y) {
            return true;
        }
    }
    
    return false;
}

void draw_game(void) {
    if (game_lost) {
        return;
    }
    
    draw_rect_filled(food_pos.x * GAME_ZOOM, food_pos.y * GAME_ZOOM, GAME_ZOOM, GAME_ZOOM, PIXEL_ON, NORM);
        
    for (size_t i = 0; i < num_snake_links; i++) {
        draw_rect(snake_links[i].x * GAME_ZOOM, snake_links[i].y * GAME_ZOOM, GAME_ZOOM, GAME_ZOOM, PIXEL_ON, NORM);
    }
}

GamePos get_rand_free_pos(void) {
    GamePos rand_pos;
    
    do
    {
        rand_pos.x = rand() % GAME_WIDTH;
        rand_pos.y = rand() % GAME_HEIGHT;
    } while (is_pos_snake(rand_pos));
    
    return rand_pos;
}

GamePos get_rel_pos(GamePos pos, int8_t dir) {
    switch (dir) {
        case UP:
            pos.y = (pos.y - 1 + GAME_HEIGHT) % GAME_HEIGHT;
            break;
        case DOWN:
            pos.y = (pos.y + 1) % GAME_HEIGHT;
            break;
        case LEFT:
            pos.x = (pos.x - 1 + GAME_WIDTH) % GAME_WIDTH;
            break;
        case RIGHT:
            pos.x = (pos.x + 1) % GAME_WIDTH;
            break;
    }
    
    return pos;
}

void add_snake_link(GamePos pos) {
    snake_links[num_snake_links] = pos;
    num_snake_links++;
}

void init_game(void) {
    free(snake_links);
    snake_links = malloc((sizeof(int)) * 10);
    
    game_lost = false;
    snake_step_timer = timer_read();
    current_snake_direction = NONE;
    desired_snake_direction = NONE;
    num_snake_links = 0;
    add_snake_link(get_rand_free_pos());
    food_pos = get_rand_free_pos();
}

void draw_snake() {
    if (snake_step_timer == 0) {
        init_game();
    }

    if (game_lost) {
        char score_str[10] = "";
        sprintf(score_str, "SCORE: %u", num_snake_links);
        draw_string(24, 12, score_str, PIXEL_ON, NORM, 1);
        
        if (desired_snake_direction != NONE && timer_elapsed(snake_step_timer) > 400) {
            init_game();
        }
    } else if (timer_elapsed(snake_step_timer) > GET_SNAKE_STEP_TIME(num_snake_links)) {
        snake_step_timer = timer_read();
        
        if (desired_snake_direction != NONE) {
            if (abs(current_snake_direction) != abs(desired_snake_direction)) {
                current_snake_direction = desired_snake_direction;
            }
            desired_snake_direction = NONE;
        }
        
        if (current_snake_direction != NONE) {
            GamePos next_pos = get_rel_pos(snake_links[0], current_snake_direction);
        
            bool ate_food = (next_pos.x == food_pos.x && next_pos.y == food_pos.y);
            
            if (ate_food) {
                num_snake_links++;
            }
                
            for (size_t i = num_snake_links - 1; i > 0; i--) {
                snake_links[i] = snake_links[i - 1];
            }
        
            if (is_pos_snake(next_pos)) {
                game_lost = true;
            }
        
            snake_links[0] = next_pos;
            
            if (ate_food) {
                food_pos = get_rand_free_pos();
            } 
        }
    }
    
    draw_game();

    send_buffer();
}

#endif

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
