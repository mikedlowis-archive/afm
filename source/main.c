#include <ncurses.h>
#include <stdbool.h>

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

int main(int argc, char** argv) {
    /* Initialize ncurses and user input settings */
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    refresh();
    /* Create the left and right windows */
    WindowLeft  = create_window(0,0,LINES,COLS/2);
    WindowRight = create_window(COLS/2,0,LINES,COLS/2);
    while(Running) {
        //getmaxyx(stdscr, Term_Y, Term_X);
        wprintw(WindowLeft,  "\rLeft");
        wprintw(WindowRight, "\rRight");
        wrefresh(WindowLeft);
        wrefresh(WindowRight);
        handle_input(getch());
    }
    destroy_window(WindowLeft);
    destroy_window(WindowRight);
    endwin();
    return 0;
}
