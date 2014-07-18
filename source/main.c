#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>

typedef struct {
    char* title;
} Window_T;

static bool Running = true;
static bool Screen_Dirty = true;

void handle_input(char ch) {
    if(ch == 'q')
        Running = false;
}

void update_screen(void) {
    /* Clear screen and update LINES and COLS */
    endwin();
    clear();
    /* Draw the Contents */
    // TODO
    /* Draw the Border */
    mvaddch(0,       0,      ACS_ULCORNER);
    mvhline(0,       1,      ACS_HLINE, COLS-2);
    mvaddch(0,       COLS-1, ACS_URCORNER);
    mvvline(1,       0,      ACS_VLINE, LINES-2);
    mvaddch(LINES-1, 0,      ACS_LLCORNER);
    mvhline(LINES-1, 1,      ACS_HLINE, COLS-2);
    mvaddch(LINES-1, COLS-1, ACS_LRCORNER);
    mvvline(1,       COLS-1, ACS_VLINE, LINES-2);
    /* Refresh and mark complete */
    refresh();
    Screen_Dirty = false;
}

void handle_signal(int sig) {
    signal(SIGWINCH, handle_signal);
    Screen_Dirty = true;
}

int main(int argc, char** argv) {
    /* Handle terminal resizing */
    signal(SIGWINCH, handle_signal);
    /* Initialize ncurses and user input settings */
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    timeout(25);
    refresh();
    while(Running) {
        if(Screen_Dirty) update_screen();
        handle_input(getch());
    }
    endwin();
    return 0;
}
