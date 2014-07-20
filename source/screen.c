#include "screen.h"
#include "state.h"
#include "aardvark.h"
#include "workdir.h"
#include <ncurses.h>

void screen_update(void) {
    /* Clear screen and update LINES and COLS */
    if(state_get_screen_resized()){
        endwin();
        state_set_screen_resized(false);
    }
    clear();
    //should probably redraw all, but since only one window exists, it doesn't matter
    workdir_ls();
    if(state_get_aardvark_mode()) aardvark_draw();
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
    state_set_screen_dirty(false);
}
