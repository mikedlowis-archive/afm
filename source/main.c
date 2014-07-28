#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "state.h"
#include "input.h"
#include "screen.h"
#include "aardvark.h"

void handle_signal(int sig) {
    state_set_screen_dirty(true);
    state_set_screen_resized(true);
}

void handle_alarm(int sig) {
	aardvark_draw();
	alarm(1);
}

int main(int argc, char** argv) {
    /* Handle terminal resizing */
    signal(SIGWINCH, handle_signal);
    signal(SIGALRM, handle_alarm);
    /* Initialize ncurses and user input settings */
    initscr();
    start_color();
    init_pair(DIRECTORY, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    raw();
    keypad(stdscr, TRUE);
    noecho();
    timeout(25);
    refresh();
    screen_init();
    state_set_mode(MODE_NORMAL);
    while(state_get_running()) {
        if(state_get_screen_dirty()) screen_update();
        input_handle_key(getch());
    }
    screen_deinit();
    clear();
    refresh();
    endwin();
    return 0;
}

