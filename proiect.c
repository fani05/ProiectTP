#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define DELAY 500000
#define JUMP_HEIGHT 13
#define GROUND_LEVEL 20
#define SCREEN_WIDTH 80

void draw_dino(int y) {
    mvprintw(y - 5, 5, "               __");
    mvprintw(y - 4, 5, "              / _)");
    mvprintw(y - 3, 5, "     _.----._/ /");
    mvprintw(y - 2, 5, "    /         /");
    mvprintw(y - 1, 5, " __/ (  | (  |");
    mvprintw(y,     5, "/__.-'|_|--|_|");
}

void draw_cactus(int x) {
    mvprintw(GROUND_LEVEL - 2, x, "  | ");
    mvprintw(GROUND_LEVEL - 1, x, "(_|_)");
    mvprintw(GROUND_LEVEL,     x, "  | ");
}

void draw_menu() {
    clear();
    mvprintw(5, SCREEN_WIDTH / 2 - 10, "Welcome to Dino Run!");
    mvprintw(7, SCREEN_WIDTH / 2 - 12, "Press SPACE to Start Game");
    mvprintw(9, SCREEN_WIDTH / 2 - 12, "Press Q to Quit");
    refresh();
}

void draw_game_over(int score, int high_score) {
    clear();
    mvprintw(10, SCREEN_WIDTH / 2 - 5, "GAME OVER!");
    mvprintw(12, SCREEN_WIDTH / 2 - 8, "Score: %d", score);
    mvprintw(13, SCREEN_WIDTH / 2 - 8, "High Score: %d", high_score);
    mvprintw(15, SCREEN_WIDTH / 2 - 10, "Press R to Replay or Q to Quit");
    refresh();
}

int main() {
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    int high_score = 0;
    int running = 1;

    while (running) {
        draw_menu();
        int ch;
        do {
            ch = getch();
            if (ch == 'q' || ch == 'Q') {
                endwin();
                return 0;
            }
            usleep(DELAY);
        } while (ch != ' ');
        nodelay(stdscr, TRUE);
        int dino_y = GROUND_LEVEL;
        int is_jumping = 0;
        int jump_counter = 0;
        int obstacle_x = SCREEN_WIDTH;
        int score = 0;
        int game_over = 0;

        while (!game_over) {
            clear();
            mvhline(GROUND_LEVEL + 1, 0, '_', SCREEN_WIDTH);
            ch = getch();
            if (ch == 'q' || ch == 'Q') {
                endwin();
                return 0;
            }
            if (ch == ' ' && !is_jumping && dino_y == GROUND_LEVEL)
                is_jumping = 1;

            if (is_jumping) {
                if (jump_counter < JUMP_HEIGHT) {
                    dino_y--;
                    jump_counter++;
                } else if (jump_counter < JUMP_HEIGHT * 2) {
                    dino_y++;
                    jump_counter++;
                } else {
                    is_jumping = 0;
                    jump_counter = 0;
                }
            }

            obstacle_x--;
            if (obstacle_x < 0) {
                obstacle_x = SCREEN_WIDTH;
                score++;
            }

            draw_dino(dino_y);
            draw_cactus(obstacle_x);
            if (obstacle_x >= 5 && obstacle_x <= 15 && dino_y >= GROUND_LEVEL - 2) {
                game_over = 1;
            }

            mvprintw(0, 0, "Score: %d", score);
            mvprintw(1, 0, "High Score: %d", high_score);
            refresh();
            usleep(DELAY);
        }

        if (score > high_score)
            high_score = score;

        nodelay(stdscr, FALSE);
        draw_game_over(score, high_score);
        while (1) {
            ch = getch();
            if (ch == 'r' || ch == 'R') break; 
            if (ch == 'q' || ch == 'Q') {
                running = 0;
                break;
            }
        }
    }

    endwin();
    return 0;
}

