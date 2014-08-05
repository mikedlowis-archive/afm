#include <signal.h>
#include <unistd.h>
#include "state.h"
#include "input.h"
#include "screen.h"

void handle_signal(int sig) {
    (void) sig;
    state_set_redraw_state(REDRAW_ALL_FRAMES);
}

void handle_alarm(int sig) {
    (void) sig;
    state_set_redraw_state(REDRAW_AARDVARK);
    alarm(1);
}

int main() {
    /* Handle terminal resizing */
    signal(SIGWINCH, handle_signal);
    signal(SIGALRM, handle_alarm);
    /* Initialize ncurses and user input settings */
    screen_init();
    state_set_mode(MODE_NORMAL);
    /* main loop */
    while(state_get_running()) {
        if(state_get_redraw_state() != REDRAW_COMPLETE)
            screen_update();
        input_handle_key(getch());
    }
    /* tear down ncurses */
    screen_deinit();
    return 0;
}

