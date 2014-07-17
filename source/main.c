#include <ncurses.h>
#include <stdbool.h>
#include <signal.h>

static bool Running = true;
//static int Term_X = 0;
//static int Term_Y = 0;
static WINDOW* WindowLeft;
static WINDOW* WindowRight;

void handle_input(char ch) {
    if(ch == 'q')
        Running = false;
}

WINDOW* create_window(int x, int y, int height, int width) {
    WINDOW* p_win = newwin(height, width, y, x);
    box(p_win, 0, 0);
    wrefresh(p_win);
    return p_win;
}

void destroy_window(WINDOW* p_win) {
    /* Erase remnants of the window */
    wborder(p_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(p_win);
    /* Delete the window */
    delwin(p_win);
}

void update_screen(void) {
    endwin();
    /* Create the left and right windows */
    WindowLeft  = create_window(0,0,LINES,COLS/2);
    wprintw(WindowLeft,  "\rLeft");
    wrefresh(WindowLeft);
    WindowRight = create_window(COLS/2,0,LINES,COLS/2);
    wprintw(WindowRight, "\rRight");
    wrefresh(WindowRight);
}

void handle_signal(int sig) {
    update_screen();
    signal(SIGWINCH, handle_signal);
}

int main(int argc, char** argv) {
    /* Handle terminal resizing */
    signal(SIGWINCH, handle_signal);
    /* Initialize ncurses and user input settings */
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    refresh();
    while(Running) {
        update_screen();
        handle_input(getch());
    }
    destroy_window(WindowLeft);
    destroy_window(WindowRight);
    endwin();
    return 0;
}
