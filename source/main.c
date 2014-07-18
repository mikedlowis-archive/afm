#include <ncurses.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>

static bool Running = true;
//static int Term_X = 0;
//static int Term_Y = 0;
static WINDOW* WindowLeft;
static WINDOW* WindowRight;

static int Idx = 0; /* TODO: this should be per-window */

void list_files(WINDOW* win) {
    FILE* ls = popen("ls", "r");
    char* filename = NULL;
    size_t len = 0;
    ssize_t read;
    int topbuff=1; /* lines to skip before printing (window border/title) */
    int i = 0;
    int rows, cols;
    getmaxyx(win, rows, cols);
    while ((read = getline(&filename, &len, ls)) != -1){
        if(i==Idx){
            wattron(win, A_STANDOUT);
            wattron(win, A_BOLD);
        }
        mvwaddnstr(win, i+topbuff, 1, filename, cols-2); /* prevent spilling out of window with long filenames */
        if(i==Idx){
            wattroff(win, A_STANDOUT);
            wattroff(win, A_BOLD);
        }
        i++;
        if(i>rows) break; /* TODO: implement scrolling to handle when there are more files than lines */
    }
    if(filename) free(filename);
}

void handle_input(char ch) {
    if(ch == 'q')
        Running = false;
    if(ch == 'j')
        Idx+=1;
    if(ch == 'k' && Idx>0)
        Idx-=1;
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
    list_files(WindowLeft);
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
