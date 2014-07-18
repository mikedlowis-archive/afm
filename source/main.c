#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>

typedef struct {
    char* title;
} Window_T;

static bool Running = true;
static bool Screen_Dirty = true;
static bool Resized = true;
static int Idx = 0; /* TODO: this should be per-window */

void list_files(void) {
    FILE* ls = popen("ls", "r");
    char* filename = NULL;
    size_t len = 0;
    ssize_t read;
    int topbuff=1; /* lines to skip before printing (window border/title) */
    int i = 0;
    while ((read = getline(&filename, &len, ls)) != -1){
        if(i==Idx){
            attron(A_STANDOUT);
            attron(A_BOLD);
        }
        mvaddnstr(i+topbuff, 1, filename, COLS-2); /* prevent spilling out of window with long filenames */
        if(i==Idx){
            attroff(A_STANDOUT);
            attroff(A_BOLD);
        }
        i++;
        if(i>LINES) break; /* TODO: implement scrolling to handle when there are more files than lines */
    }
    if(filename) free(filename);
}

void update_screen(void) {
    /* Clear screen and update LINES and COLS */
    if(Resized) {
        endwin();
        Resized = false;
    }
    clear();
    /* Draw the Contents */
    list_files();
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

void handle_input(char ch) {
    /* Assume screen is dirty by default */
    bool is_screen_dirty = true;
    /* Handle the key press */
    switch(ch) {
        case 'q': Running = false;
                  break;
        case 'j': Idx += 1;
                  break;
        case 'k': Idx -= 1;
                  break;
        default:  is_screen_dirty = false; // Screen is not actually dirty
                  break;
    };
    Screen_Dirty = Screen_Dirty || is_screen_dirty;
}

void handle_signal(int sig) {
    signal(SIGWINCH, handle_signal);
    Screen_Dirty = true;
    Resized = true;
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
