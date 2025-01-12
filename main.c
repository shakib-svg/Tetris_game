#include <stdio.h>
#include <string.h>
#include <time.h>
#include "minirisc.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "harvey_platform.h"
#include "xprintf.h"
//////////////////////////
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
/////////////////////////
#define NUM_SHAPES 7
/*The board may be any size, although the standard Tetris board is 10 wide and 20 high*/
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define SQUARE_SIZE 20
///////////////////
static uint32_t frame_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];// 640x480 screen res 
volatile uint32_t color = 0x00ff0000;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tetromino shapes with 4 rotations for each // 1 cte color for each 
int shapes[NUM_SHAPES][4][4][2] = {
    // I-shape (4 rotations)
    {
        {{0,0}, {1,0}, {2,0}, {3,0}},
        {{1,0}, {1,1}, {1,2}, {1,3}},
        {{0,1}, {1,1}, {2,1}, {3,1}},
        {{2,0}, {2,1}, {2,2}, {2,3}}
    },
    // O-shape (pas de rotation)
    {
        {{0,0}, {1,0}, {0,1}, {1,1}},
        {{0,0}, {1,0}, {0,1}, {1,1}},
        {{0,0}, {1,0}, {0,1}, {1,1}},
        {{0,0}, {1,0}, {0,1}, {1,1}}
    },
    // T-shape (4 rotations)
    {
        {{1,0}, {0,1}, {1,1}, {2,1}},
        {{1,0}, {1,1}, {2,1}, {1,2}},
        {{0,1}, {1,1}, {2,1}, {1,2}},
        {{1,0}, {0,1}, {1,1}, {1,2}}
    },
    // L-shape (4 rotations)
    {
        {{0,0}, {0,1}, {0,2}, {1,2}},
        {{0,1}, {1,1}, {2,1}, {2,0}},
        {{1,0}, {2,0}, {2,1}, {2,2}},
        {{0,2}, {1,2}, {2,2}, {2,1}}
    },
    // Reverse L-shape
    {
        {{1,0}, {1,1}, {1,2}, {0,2}},
        {{0,0}, {0,1}, {1,1}, {2,1}},
        {{1,0}, {2,0}, {1,1}, {1,2}},
        {{0,1}, {1,1}, {2,1}, {2,2}}
    },
    // S-shape
    {
        {{1,0}, {2,0}, {0,1}, {1,1}},
        {{0,0}, {0,1}, {1,1}, {1,2}},
        {{1,0}, {2,0}, {0,1}, {1,1}},
        {{0,0}, {0,1}, {1,1}, {1,2}}
    },
    // Z-shape
    {
        {{0,0}, {1,0}, {1,1}, {2,1}},
        {{1,0}, {0,1}, {1,1}, {0,2}},
        {{0,0}, {1,0}, {1,1}, {2,1}},
        {{1,0}, {0,1}, {1,1}, {0,2}}
    }
};

// Shape colors for variety
uint32_t shape_colors[NUM_SHAPES] = {
    0xFF00FFFF,  // Cyan for I
    0xFFFFFF00,  // Yellow for O
    0xFF800080,  // Purple for T
    0xFFFFA500,  // Orange for L
    0xFF0000FF,  // Blue for Reverse L
    0xFF00FF00,  // Green for S
    0xFFFF0000   // Red for Z
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Game state structure
struct {
    int board[BOARD_WIDTH][BOARD_HEIGHT]; // size standart de tetris 
    int current_shape[4][2]; 
    int current_shape_type;
    int current_rotation;
    int current_x, current_y;
    int score;
    int level;
    int lines_cleared;
} game_state;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_video()
{
    memset(frame_buffer, 0, sizeof(frame_buffer)); // clear frame buffer to black
    VIDEO->WIDTH  = SCREEN_WIDTH;
    VIDEO->HEIGHT = SCREEN_HEIGHT;
    VIDEO->DMA_ADDR = frame_buffer;
    VIDEO->CR = VIDEO_CR_IE | VIDEO_CR_EN;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw_square(int x, int y, int width, uint32_t color)
{
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
        return;
    }
    int i, j;
    int x_start = x < 0 ? 0 : x; // si x plus petit que 0 x_start = 0 sinon x=x 
    int y_start = y < 0 ? 0 : y;//..
    int x_end = x + width;
    int y_end = y + width;
    if (x_end > SCREEN_WIDTH) {
        x_end = SCREEN_WIDTH;
    }
    if (y_end > SCREEN_HEIGHT) {
        y_end = SCREEN_HEIGHT;
    }
    for (j = y_start; j < y_end; j++) {
        for (i = x_start; i < x_end; i++) {
            frame_buffer[j*SCREEN_WIDTH + i] = color;
        }
    }
}

// Draw a grid for the Tetris board
void draw_board_grid() {
    for (int y = 0; y <= BOARD_HEIGHT; y++) {
        int screen_y = y * SQUARE_SIZE;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            frame_buffer[screen_y * SCREEN_WIDTH + x] = 0xFF333333;
        }
    }
    
    for (int x = 0; x <= BOARD_WIDTH; x++) {
        int screen_x = x * SQUARE_SIZE;
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            frame_buffer[y * SCREEN_WIDTH + screen_x] = 0xFF333333;
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void rotate_shape() {
    int new_rotation = (game_state.current_rotation + 1) % 4;
    int can_rotate = 1;
    
    // Check if rotation is possible
    for (int i = 0; i < 4; i++) {
        int new_x = game_state.current_x + shapes[game_state.current_shape_type][new_rotation][i][0];
        int new_y = game_state.current_y + shapes[game_state.current_shape_type][new_rotation][i][1];
        
        // Check for board boundaries and collisions
        if (new_x < 0 || new_x >= BOARD_WIDTH || 
            new_y < 0 || new_y >= BOARD_HEIGHT ||
            (new_y >= 0 && game_state.board[new_x][new_y])) {
            can_rotate = 0;
            break;
        }
    }
    
    // Apply rotation if possible
    if (can_rotate) {
        game_state.current_rotation = new_rotation;
        memcpy(game_state.current_shape, 
               shapes[game_state.current_shape_type][new_rotation], 
               sizeof(game_state.current_shape));
    }
}


int can_move(int dx, int dy) {
    for (int i = 0; i < 4; i++) {
        int new_x = game_state.current_x + game_state.current_shape[i][0] + dx;
        int new_y = game_state.current_y + game_state.current_shape[i][1] + dy;
        
        // Check boundaries
        if (new_x < 0 || new_x >= BOARD_WIDTH || 
            new_y >= BOARD_HEIGHT) {
            return 0;
        }
        
        // Check board collisions, ignore checks above board
        if (new_y >= 0 && game_state.board[new_x][new_y]) {
            return 0;
        }
    }
    return 1;
}

void move_shape(int dx, int dy) {
    if (can_move(dx, dy)) {
        game_state.current_x += dx;
        game_state.current_y += dy;
    } else if (dy > 0) {
        // Piece has landed, add to board
        for (int i = 0; i < 4; i++) {
            int x = game_state.current_x + game_state.current_shape[i][0];
            int y = game_state.current_y + game_state.current_shape[i][1];
            
            if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
                game_state.board[x][y] = game_state.current_shape_type + 1;
            }
        }
        
        // Spawn new shape
        spawn_shape();
    }
}

void spawn_shape() {
    // Randomly select a shape
    game_state.current_shape_type = rand() % NUM_SHAPES;
    game_state.current_rotation = 0;
    
    // Start at top center
    game_state.current_x = BOARD_WIDTH / 2 - 2;
    game_state.current_y = 0;
    
    // Check for game over
    for (int i = 0; i < 4; i++) {
        int x = game_state.current_x + shapes[game_state.current_shape_type][0][i][0];
        int y = game_state.current_y + shapes[game_state.current_shape_type][0][i][1];
        
        if (game_state.board[x][y]) {
            // Game over reset
            memset(&game_state, 0, sizeof(game_state));
        }
    }
    
    // Copy initial shape configuration
    memcpy(game_state.current_shape, 
           shapes[game_state.current_shape_type][0], 
           sizeof(game_state.current_shape));
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void check_line_clear() {
    int lines_cleared = 0;
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        int line_full = 1;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (!game_state.board[x][y]) {
                line_full = 0;
                break;
            }
        }
        
        if (line_full) {
            // Remove the line and shift down
            for (int ny = y; ny > 0; ny--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    game_state.board[x][ny] = game_state.board[x][ny-1];
                }
            }
            
            // Clear top line
            for (int x = 0; x < BOARD_WIDTH; x++) {
                game_state.board[x][0] = 0;
            }
            
            lines_cleared++;
            y++; // Recheck this line as it's now a new line
        }
    }
    
    // Update score
    if (lines_cleared > 0) {
        static int score_multiplier[] = {0, 40, 100, 300, 1200};
        game_state.score += score_multiplier[lines_cleared] * (game_state.level + 1);
        game_state.lines_cleared += lines_cleared;
        game_state.level = game_state.lines_cleared / 10;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
volatile int refresh_event = 0;

void video_interrupt_handler()
{
    VIDEO->SR = 0;
    refresh_event = 1;
}

void keyboard_interrupt_handler()
{
    uint32_t kdata;
    while (KEYBOARD->SR & KEYBOARD_SR_FIFO_NOT_EMPTY) {
        kdata = KEYBOARD->DATA;
        if (kdata & KEYBOARD_DATA_PRESSED) {
			xprintf("%d\n",KEYBOARD_KEY_CODE(kdata));
            switch (KEYBOARD_KEY_CODE(kdata)) {
                case 27: // Q - Quit
                    minirisc_halt();
                    break;
                case 32: // Space - Rotate
                    rotate_shape();
                    break;
                case 80: // Left arrow
                    move_shape(-1, 0);
                    break;
                case 79: // Right arrow
                    move_shape(1, 0);
                    break;
                case 81: // Down arrow - Soft drop
                    move_shape(0, 1);
                    break;
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw_current_shape() {
    uint32_t current_color = shape_colors[game_state.current_shape_type];
    for (int i = 0; i < 4; i++) {
        int sx = (game_state.current_x + game_state.current_shape[i][0]) * SQUARE_SIZE;
        int sy = (game_state.current_y + game_state.current_shape[i][1]) * SQUARE_SIZE;
        draw_square(sx, sy, SQUARE_SIZE, current_color);
    }
}

void draw_static_board() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (game_state.board[x][y]) {
                draw_square(x * SQUARE_SIZE, y * SQUARE_SIZE, 
                            SQUARE_SIZE, 
                            shape_colors[game_state.board[x][y] - 1]);
            }
        }
    }
}

void draw_score() {
    // Simple text drawing (this would require a font implementation)
    xprintf("Score: %d\n", game_state.score);
    xprintf("Level: %d\n", game_state.level);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    init_video();
    
    // Initialize game state
    memset(&game_state, 0, sizeof(game_state));
    srand(0); // Simple seed for random shape generation
    
    spawn_shape();
    
    // Enable interrupts
    KEYBOARD->CR |= KEYBOARD_CR_IE;
    minirisc_enable_interrupt(VIDEO_INTERRUPT |KEYBOARD_INTERRUPT);
    minirisc_enable_global_interrupts();
    
    // Game loop
    int drop_timer = 0;
    int drop_speed = 30; // Adjustable drop speed
    
    while (1) {
        minirisc_wait_for_interrupt();
        
        if (refresh_event) {
            refresh_event = 0;
            memset(frame_buffer, 0, sizeof(frame_buffer));
            
            // Draw board grid
            draw_board_grid();
            
            // Game logic: periodic shape drop
            drop_timer++;
            if (drop_timer >= drop_speed) { // Adjust for game speed
                drop_timer = 0;
                move_shape(0, 1);
            }
            
            // Check for completed lines
            check_line_clear();
            
            // Draw static board pieces
            draw_static_board();
            
            // Draw current falling shape
            draw_current_shape();
            
            // Draw score (would need font implementation)
            draw_score();
        }
    }
    
    return 0;
}





