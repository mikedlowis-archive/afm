#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> /*TODO: anything using this is almost certainly broken on windows */
#include <string.h>

#include "aardvark.h"
#include "state.h"
#include "input.h"
#include "screen.h"
#include "workdir.h"

void handle_signal(int sig) {
    signal(SIGWINCH, handle_signal);
    state_set_screen_dirty(true);
    state_set_screen_resized(true);
}

int main(int argc, char** argv) {
    workdir_init(0);
    /* Handle terminal resizing */
    signal(SIGWINCH, handle_signal);
    /* Initialize ncurses and user input settings */
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    timeout(25);
    refresh();
    screen_init();
    while(state_get_running()) {
        if(state_get_screen_dirty()) screen_update();
        input_handle_key(getch());
    }
    erase();
    refresh();
    endwin();
    screen_deinit();
    workdir_deinit();
    return 0;
}

