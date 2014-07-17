#include <ncurses.h>
#include <stdbool.h>

static bool Running = true;
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
    WindowLeft  = create_window(0,0,20,20);
    WindowRight = create_window(20,0,20,20);
    while(Running) {
        wprintw(WindowLeft,  "Left");
        wprintw(WindowRight, "Right");
        wrefresh(WindowLeft);
        wrefresh(WindowRight);
        handle_input(getch());
    }
    destroy_window(WindowLeft);
    destroy_window(WindowRight);
    endwin();
    return 0;
}
