#include <ncurses.h>
#include <stdbool.h>
#include <signal.h>

typedef struct {
    char* title;
} Window_T;

static bool Running = true;
static bool Screen_Dirty = true;
Window_T Windows[2] = {
    { "Left" },
    { "Right" }
};

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

/*
 * This routine draws a frame around a specified part of the console
 * screen, using special characters such as ACS_ULCORNER and ACS_VLINE.
 * The frame extends from startrow to endrow vertically and from
 * startcol to endcol horizontally.  (Rows and columns are numbered
 * starting from 0.)  Note that the interior of the frame extends
 * from startrow+1 to endrow-1 vertically and from startcol+1 to
 * endcol-1 horizontally.
 */
void drawFrame(char* p_label, int startrow, int startcol, int endrow, int endcol) {
   //int saverow, savecol;
   //getyx(stdscr,saverow,savecol);
   //mvaddch(startrow,startcol,ACS_ULCORNER);
   //for (int i = startcol + 1; i < endcol; i++)
   //   addch(ACS_HLINE);
   //addch(ACS_URCORNER);
   //for (int i = startrow +1; i < endrow; i++) {
   //   mvaddch(i,startcol,ACS_VLINE);
   //   mvaddch(i,endcol,ACS_VLINE);
   //}
   //mvaddch(endrow,startcol,ACS_LLCORNER);
   //for (int i = startcol + 1; i < endcol; i++)
   //   addch(ACS_HLINE);
   //addch(ACS_LRCORNER);
   //move(saverow,savecol);
   //refresh();
}

void update_screen(void) {
    int rows, cols;
    endwin();
    clear();
    getmaxyx(stdscr,rows,cols);
    //move(5,5);      printw("%d %d",rows,cols);
    mvaddch(0,      0,      ACS_ULCORNER);
    mvhline(0,      1,      ACS_HLINE, cols-2);
    mvaddch(0,      cols-1, ACS_URCORNER);
    mvvline(1,      0,      ACS_VLINE, rows-2);
    mvaddch(rows-1, 0,      ACS_LLCORNER);
    mvhline(rows-1, 1,      ACS_HLINE, cols-2);
    mvaddch(rows-1, cols-1, ACS_LRCORNER);
    mvvline(1,      cols-1, ACS_VLINE, rows-2);
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
    destroy_window(WindowLeft);
    destroy_window(WindowRight);
    endwin();
    return 0;
}
