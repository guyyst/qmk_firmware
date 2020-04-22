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
#ifdef ENABLE_SNAKE_MODE
        case OLED_GOF:
            draw_game_of_life();
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


#ifdef ENABLE_SNAKE_MODE

uint8_t zoom_index = 3;
#define GAME_ZOOM (1 << zoom_index)
#define GAME_WIDTH (128 / GAME_ZOOM)
#define GAME_HEIGHT (32 / GAME_ZOOM)

void change_snake_zoom(bool increase) {
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

typedef struct SnakeLink SnakeLink;
struct SnakeLink {
    GamePos pos;
    SnakeLink *next;
    SnakeLink *prev;
};

GamePos food_pos;

SnakeLink *snake_head = NULL;
SnakeLink *snake_tail = NULL;

uint16_t score = 0;

int8_t current_snake_direction = NONE;
int8_t desired_snake_direction = NONE;

bool game_lost = false;

bool is_pos_snake(GamePos pos) {
    for (SnakeLink *it = snake_head; it != NULL; it = it->next) {
        if (it->pos.x == pos.x && it->pos.y == pos.y) {
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

    for (SnakeLink *it = snake_head; it != NULL; it = it->next) {
        draw_rect(it->pos.x * GAME_ZOOM, it->pos.y * GAME_ZOOM, GAME_ZOOM, GAME_ZOOM, PIXEL_ON, NORM);
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

void grow_snake(GamePos pos) {
    SnakeLink *link = malloc(sizeof(SnakeLink));
    link->pos = pos;
    link->next = NULL;
    link->prev = snake_tail;
    
    if (link->prev != NULL) {
        link->prev->next = link;
    }
    
    if (snake_head == NULL) {
        snake_head = link;
    }
    
    snake_tail = link;
    
    score++;
}

void clear_snake(void) {
    SnakeLink *it = snake_head;
    
    while (it != NULL)
    {
        SnakeLink *temp = it;
        it = it->next;
        free(temp);
    }
    
    snake_head = NULL;
    snake_tail = NULL;
}

void init_game(void) {
    clear_snake();
    score = 0;
    game_lost = false;
    snake_step_timer = timer_read();
    current_snake_direction = NONE;
    desired_snake_direction = NONE;
    grow_snake(get_rand_free_pos());
    food_pos = get_rand_free_pos();
}

void draw_snake() {
    if (snake_step_timer == 0) {
        init_game();
    }

    if (game_lost) {
        char score_str[10] = "";
        sprintf(score_str, "SCORE: %u", score);
        draw_string(24, 12, score_str, PIXEL_ON, NORM, 1);

        if (desired_snake_direction != NONE && timer_elapsed(snake_step_timer) > 400) {
            init_game();
        }
    } else if (timer_elapsed(snake_step_timer) > GET_SNAKE_STEP_TIME(score)) {
        snake_step_timer = timer_read();

        if (desired_snake_direction != NONE) {
            if (abs(current_snake_direction) != abs(desired_snake_direction)) {
                current_snake_direction = desired_snake_direction;
            }
            desired_snake_direction = NONE;
        }

        if (current_snake_direction != NONE) {
            GamePos next_pos = get_rel_pos(snake_head->pos, current_snake_direction);
        
            GamePos old_tail_pos = snake_tail->pos;
    
            for (SnakeLink *it = snake_tail; it != snake_head; it = it->prev) {
                it->pos = it->prev->pos;
            }
            
            if (is_pos_snake(next_pos)) {
                game_lost = true;
            }
            
            snake_head->pos = next_pos;

            bool ate_food = (next_pos.x == food_pos.x && next_pos.y == food_pos.y);
            
            if (ate_food) {
                grow_snake(old_tail_pos);
                food_pos = get_rand_free_pos();
            }
        }
    }

    draw_game();

    send_buffer();
}

#endif

#ifdef ENABLE_GAME_OF_LIFE

// void change_gof_zoom(bool increase) {
//     if (increase && zoom_index < 4) {
//         zoom_index++;
//     } else if (!increase && zoom_index > 0) {
//         zoom_index--;
//     }

//     init_game();
// }

#    define WIDTH 128
#    define HEIGHT 32
#    define PIXEL_ARRAY_SIZE (WIDTH * HEIGHT / 8)

uint8_t *gof_array_cur;
uint8_t *gof_array_next;

uint16_t gof_generation = 0;

uint16_t gof_gen_timer = 0;


bool gof_get_pixel_state(uint8_t *array, uint8_t col, uint8_t row) {
    uint16_t pixel_pos = row * WIDTH + col;

    uint16_t byte_pos = pixel_pos / 8;
    uint8_t  bit_pos  = 7 - (pixel_pos % 8);

    return array[byte_pos] & (1 << bit_pos);
}

void gof_set_pixel_state(uint8_t *array, uint8_t col, uint8_t row, bool state) {
    uint16_t pixel_pos = row * WIDTH + col;

    uint16_t byte_pos = pixel_pos / 8;
    uint8_t  bit_pos  = 7 - (pixel_pos % 8);

    if (state) {
        array[byte_pos] |= (1 << bit_pos);
    } else {
        array[byte_pos] &= ~(1 << bit_pos);
    }
}

void gof_draw_current_array(void) {
    for (uint8_t row = 0; row < HEIGHT; row++) {
        for (uint8_t col = 0; col < WIDTH; col++) {
            if (gof_get_pixel_state(gof_array_cur, col, row)) {
                draw_pixel(col, row, PIXEL_ON, NORM);
            }
        }
    }
}

void gof_fill_random(void) {
    for (uint8_t row = 0; row < HEIGHT; row++) {
        for (uint8_t col = 0; col < WIDTH; col++) {
            if (rand() % 7 == 0) {
                gof_set_pixel_state(gof_array_cur, col, row, true);
            }
        }
    }
}

uint8_t gof_mod(uint8_t number, uint8_t mod) {
    return (number + mod) % mod;
}

void gof_step(void) {

    for (uint8_t row = 0; row < HEIGHT; row++) {
        for (uint8_t col = 0; col < WIDTH; col++) {
            
            uint8_t live_neighbors = 0;

            for (int8_t y = -1; y <= 1; y++) {
                for (int8_t x = -1; x <= 1; x++) {

                    if (y == 0 && x == 0) {
                        continue;
                    }
                    
                    live_neighbors += gof_get_pixel_state(gof_array_cur, gof_mod(col + x, WIDTH), gof_mod(row + y, HEIGHT));
                }
            }

            bool cell_state = gof_get_pixel_state(gof_array_cur, col, row);

            if ((cell_state && (live_neighbors == 2 || live_neighbors == 3)) || (!cell_state && live_neighbors == 3)) {
                gof_set_pixel_state(gof_array_next, col, row, true);
            }
            else {
                gof_set_pixel_state(gof_array_next, col, row, false);
            }
        }
    }

    uint8_t *gof_array_temp = gof_array_cur;
    gof_array_cur = gof_array_next;
    gof_array_next = gof_array_temp;

    ++gof_generation;
}

void gof_init(void) {

    gof_generation = 1;

    free(gof_array_cur);
    free(gof_array_next);

    gof_array_cur = calloc(PIXEL_ARRAY_SIZE, 1);
    gof_array_next = calloc(PIXEL_ARRAY_SIZE, 1);

    gof_gen_timer = timer_read();
}   

void draw_game_of_life() {
    if (gof_generation == 0) {
        gof_init();
     
        gof_set_pixel_state(gof_array_cur, 40, 10, true);
        gof_set_pixel_state(gof_array_cur, 41, 10, true);
        gof_set_pixel_state(gof_array_cur, 42, 10, true);
        gof_set_pixel_state(gof_array_cur, 43, 10, true);
        gof_set_pixel_state(gof_array_cur, 44, 10, true);
        gof_set_pixel_state(gof_array_cur, 45, 10, true);
        gof_set_pixel_state(gof_array_cur, 46, 10, true);
        gof_set_pixel_state(gof_array_cur, 47, 10, true);
        gof_set_pixel_state(gof_array_cur, 48, 10, true);
        gof_set_pixel_state(gof_array_cur, 49, 10, true);
        gof_set_pixel_state(gof_array_cur, 50, 10, true);
        gof_set_pixel_state(gof_array_cur, 51, 10, true);
        gof_set_pixel_state(gof_array_cur, 52, 10, true);
        gof_set_pixel_state(gof_array_cur, 53, 10, true);
        gof_set_pixel_state(gof_array_cur, 54, 10, true);
        gof_set_pixel_state(gof_array_cur, 55, 10, true);
        gof_set_pixel_state(gof_array_cur, 56, 10, true);
        gof_set_pixel_state(gof_array_cur, 57, 10, true);
        gof_set_pixel_state(gof_array_cur, 58, 10, true);
        gof_set_pixel_state(gof_array_cur, 59, 10, true);
        gof_set_pixel_state(gof_array_cur, 60, 10, true);
        gof_set_pixel_state(gof_array_cur, 61, 10, true);
        gof_set_pixel_state(gof_array_cur, 62, 10, true);
        gof_set_pixel_state(gof_array_cur, 63, 10, true);
        gof_set_pixel_state(gof_array_cur, 64, 10, true);
        gof_set_pixel_state(gof_array_cur, 65, 10, true);
        gof_set_pixel_state(gof_array_cur, 66, 10, true);
        gof_set_pixel_state(gof_array_cur, 67, 10, true);
        gof_set_pixel_state(gof_array_cur, 68, 10, true);
        gof_set_pixel_state(gof_array_cur, 69, 10, true);
        gof_set_pixel_state(gof_array_cur, 70, 10, true);


        gof_set_pixel_state(gof_array_cur, 40, 20, true);
        gof_set_pixel_state(gof_array_cur, 41, 20, true);
        gof_set_pixel_state(gof_array_cur, 42, 20, true);
        gof_set_pixel_state(gof_array_cur, 43, 20, true);
        gof_set_pixel_state(gof_array_cur, 44, 20, true);
        gof_set_pixel_state(gof_array_cur, 45, 20, true);
        gof_set_pixel_state(gof_array_cur, 46, 20, true);
        gof_set_pixel_state(gof_array_cur, 47, 20, true);
        gof_set_pixel_state(gof_array_cur, 48, 20, true);
        gof_set_pixel_state(gof_array_cur, 49, 20, true);
        gof_set_pixel_state(gof_array_cur, 50, 20, true);
        gof_set_pixel_state(gof_array_cur, 51, 20, true);
        gof_set_pixel_state(gof_array_cur, 52, 20, true);
        gof_set_pixel_state(gof_array_cur, 53, 20, true);
        gof_set_pixel_state(gof_array_cur, 54, 20, true);
        gof_set_pixel_state(gof_array_cur, 55, 20, true);
        gof_set_pixel_state(gof_array_cur, 56, 20, true);
        gof_set_pixel_state(gof_array_cur, 57, 20, true);
        gof_set_pixel_state(gof_array_cur, 58, 20, true);
        gof_set_pixel_state(gof_array_cur, 59, 20, true);
        gof_set_pixel_state(gof_array_cur, 60, 20, true);
        gof_set_pixel_state(gof_array_cur, 61, 20, true);
        gof_set_pixel_state(gof_array_cur, 62, 20, true);
        gof_set_pixel_state(gof_array_cur, 63, 20, true);
        gof_set_pixel_state(gof_array_cur, 64, 20, true);
        gof_set_pixel_state(gof_array_cur, 65, 20, true);
        gof_set_pixel_state(gof_array_cur, 66, 20, true);
        gof_set_pixel_state(gof_array_cur, 67, 20, true);
        gof_set_pixel_state(gof_array_cur, 68, 20, true);
        gof_set_pixel_state(gof_array_cur, 69, 20, true);
        gof_set_pixel_state(gof_array_cur, 70, 20, true);
        // gof_fill_random();
    }

    if (timer_elapsed(gof_gen_timer) > 1) {
        gof_gen_timer = timer_read();

        gof_step();
        gof_draw_current_array();
        send_buffer();
    }
}

#endif

