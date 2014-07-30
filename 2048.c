#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>

#define LEFT 1
#define RIGHT 2
#define UP 3 
#define DOWN 4 

#define BROWN 255
#define WHITE 254
#define BLACK 253

#define SIZE 4

int board[SIZE][SIZE];
int score = 0;

int addTile(void);
void moveBoard(short dir);
int indexBoard(int y, int x, short dir);
void setBoard(int y, int x, short dir, int val);
void drawBoard();
bool gameOver(void);
void setupColors();

int main() {

    srand((unsigned int)time(NULL));

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = 0;
        }
    }

    initscr();
    setupColors();
    keypad(stdscr, true);
    curs_set(0);
    noecho();
    raw();

    addTile();
    addTile();

    int c;
    drawBoard();
    
    while ((c = getch()) != 'Q') {
        switch (c) {
            case KEY_UP:
            case 'k':
            case 'w':
                moveBoard(UP);
                break;
            case KEY_DOWN:
            case 'j':
            case 's':
                moveBoard(DOWN);
                break;
            case KEY_LEFT:
            case 'h':
            case 'a':
                moveBoard(LEFT);
                break;
            case KEY_RIGHT:
            case 'l':
            case 'd':
                moveBoard(RIGHT);
                break;
        }

        erase();
        drawBoard();

        if (gameOver()) {
            attron(A_BOLD | COLOR_PAIR(WHITE));
            mvprintw(((LINES/2)-7) + (SIZE*3), (COLS/2)-10, "GAME OVER");
            attroff(A_BOLD | COLOR_PAIR(WHITE));
            refresh();
            sleep(1);
            flushinp();
            getch();
            break;
        }
    }
    
    endwin();

    return 0;
}

int addTile(void) {
    int *spaces[SIZE*SIZE], index = 0;
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == 0) {
                spaces[index++] = &board[i][j];
            }
        }
    }

    if (index == 0) {
        return -1;
    }

    //1 in 10 chance of a 4 (0 is a #)
    *spaces[rand()%index] = (rand()%10 > 8) ? 4: 2;

    return 0;
}

void drawBoard(void) {
    int basex = (COLS/2)-10;
    int basey = (LINES/2)-7;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            move(basey + i*3, basex + j*5);
            if (board[i][j] != 0) {
                attron(A_BOLD | COLOR_PAIR((int)log2(board[i][j])+15));

                printw("     ");
                move(basey + (i*3)+1, basex + j*5);
                printw("%4d ", board[i][j]);
                move(basey + (i*3)+2, basex + j*5);
                printw("     ");

                attroff(A_BOLD | COLOR_PAIR((int)log2(board[i][j])+15));
            }
            else {
                move(basey + (i*3)+1, basex + (j*5)+3);
                attron(COLOR_PAIR(WHITE));
                printw(".");
                attroff(COLOR_PAIR(WHITE));
            }
        }
    }
    attron(COLOR_PAIR(WHITE));
    mvprintw(basey + (SIZE*3)+1, basex, "%d pts", score);
    attroff(COLOR_PAIR(WHITE));
}

void moveBoard(short dir) {

    bool wasValidMove = false;

    //tile merging
    //it's a bit weird because it hinges on index/setBoard
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE-1; y++) {

            int tile = indexBoard(y, x, dir);
            int next, skip = 1; //next lines skip zeroes
            do {
                next = indexBoard(y+skip, x, dir);
                skip++;
            }  while (next == 0 && y+skip < SIZE);

            if (tile == next && tile != 0) {
                setBoard(y+1, x, dir, 0);
                setBoard(y, x, dir, tile+next);
                score += tile+next;
                wasValidMove = true;
            }
        }
    }

    //sliding
    //builds a buffer ignoring 0s, then replaces the gird section with the buf
    int line[SIZE], li;
    for (int x = 0; x < SIZE; x++) {
        li = 0;
        for (int y = 0; y < SIZE; y++) {
            int tile = indexBoard(y, x, dir);
            if (tile != 0) {
                if (li < y) {
                    wasValidMove = true;
                }
                line[li++] = tile;
            }
        }
        for (int y = 0; y < SIZE; y++) {
            if (y < li) {
                setBoard(y, x, dir, line[y]);
            }
            else {
                setBoard(y, x, dir, 0);
            }
        }
    }

    if (wasValidMove) {
        addTile();
    }
}

//indexes the board given the direction you're looking at it
int indexBoard(int y, int x, short dir) {
    switch (dir) {
        case DOWN:
            return board[SIZE-1-y][x];
            break;
        case UP:
            return board[y][x];
            break;
        case LEFT:
            return board[SIZE-1-x][y];
            break;
        case RIGHT:
            return board[SIZE-1-x][SIZE-1-y];
            break;
    }
    return -1;
}

void setBoard(int y, int x, short dir, int val) {
    switch (dir) {
        case DOWN:
            board[SIZE-1-y][x] = val;
            break;
        case UP:
            board[y][x] = val;
            break;
        case LEFT:
            board[SIZE-1-x][y] = val;
            break;
        case RIGHT:
            board[SIZE-1-x][SIZE-1-y] = val;
            break;
    }
    return;
}

bool gameOver(void) {
    for (int x = 0; x < SIZE-1; x++) {
        for (int y = 0; y < SIZE-1; y++) {
            int tile = board[y][x];
            int below = board[y+1][x];
            int next = board[y][x+1];
            if (tile == 0 || below == 0 || next == 0 || tile == below || tile == next) {
                return false;
            }
        }
        if (board[SIZE-1][x] == board[SIZE-1][x+1]) {
            return false;
        }
    }
    for (int y = 0; y < SIZE-1; y++) {
        if (board[y][SIZE-1] == 0 || board[y][SIZE-1] == board[y+1][SIZE-1]) {
            return false;
        }
    }
    if (board[SIZE-1][SIZE-1] == 0) {
        return false;
    }
    
    return true; //ah!
    
}

void setupColors() {
    start_color();
    
    init_color(BROWN, 465, 430, 394);
    init_color(WHITE, 973, 961, 946);
    init_color(BLACK, 0, 0, 0);

    init_pair(WHITE, WHITE, COLOR_BLACK);

    init_color(1+15, 930, 891, 852);
    init_pair(1+15, BROWN, 1+15);

    init_color(2+15, 926, 875, 782);
    init_pair(2+15, BROWN, 2+15);

    init_color(3+15, 946, 692, 473);
    init_pair(3+15, WHITE, 3+15);

    init_color(4+15, 957, 582, 387);
    init_pair(4+15, WHITE, 4+15);

    init_color(5+15, 961, 484, 371);
    init_pair(5+15, WHITE, 5+15);

    init_color(6+15, 961, 367, 230);
    init_pair(6+15, WHITE, 6+15);

    init_color(7+15, 926, 809, 445);
    init_pair(7+15, BLACK, 7+15);

    init_color(8+15, 926, 797, 379);
    init_pair(8+15, BLACK, 8+15);

    init_color(9+15, 926, 782, 312);
    init_pair(9+15, BLACK, 9+15);

    init_color(10+15, 926, 770, 246);
    init_pair(10+15, BLACK, 10+15);

    init_color(11+15, 926, 758, 179);
    init_pair(11+15, BLACK, 11+15);

    init_color(12+15, 234, 226, 195);

    for (int i = 12+15; i < 18+15; i++) {
        init_pair(i, WHITE, 12+15);
    }
}
