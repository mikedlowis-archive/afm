#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "input.h"
#include "state.h"
#include "workdir.h"
#include "screen.h"
#include "frame.h"

#define ESC (27)

#define KEY_BUFFER_LENGTH (1024)

static size_t Key_Buffer_Size = 0;

static char* Key_Buffer = NULL;

//keybinding function callback
typedef void (*key_cb_t)(void);

typedef struct {
    char* sequence;
    key_cb_t callback;
} binding_t;

static void handle_aardvark(void) {
    state_set_aardvark_mode(!state_get_aardvark_mode());
    state_set_refresh_state(REFRESH_AARDVARK);
}

static void handle_quit(void) {
    state_set_running(false);
}

static void handle_next(void) {
    workdir_next(state_get_focused_workdir());
}

static void handle_prev(void) {
    workdir_prev(state_get_focused_workdir());
}

static void handle_cd(void) {
    workdir_cd(state_get_focused_workdir());
}

static void handle_scroll_to_top(void) {
    workdir_scroll_to_top(state_get_focused_workdir());
}

static void handle_scroll_to_bottom(void) {
    workdir_scroll_to_bot(state_get_focused_workdir());
}

static void handle_page_up(void){
    frame_page_up(state_get_focused_frame());
}

static void handle_page_down(void){
    frame_page_down(state_get_focused_frame());
}

static void handle_expand(void){
    workdir_expand_selected(state_get_focused_workdir());
}

static void handle_collapse(void){
    workdir_collapse_selected(state_get_focused_workdir());
}

void handle_force_redraw(void){
    state_set_refresh_state(REFRESH_ALL_WINS);
}

void handle_search_mode(void){
    state_set_mode(MODE_SEARCH);
}

static binding_t Default_Bindings[] = {
    { "a",   &handle_aardvark },
    { "q",   &handle_quit },
    { "j",   &handle_next },
    { "k",   &handle_prev },
    { "\n",  &handle_cd },
    { "wn",  &screen_open },
    { "wc",  &screen_close },
    { "/",   &handle_search_mode },
    { "gg",  &handle_scroll_to_top },
    { "G",   &handle_scroll_to_bottom },
    { "U",   &handle_page_up },
    { "D",   &handle_page_down },
    { "l",   &handle_expand },
    { "h",   &handle_collapse },
    { "wj",  &screen_focus_next },
    { "wk",  &screen_focus_prev },
    { "wm",  &screen_focus_master },
    { "w\n", &screen_swap_with_master },
    { "wJ", &screen_swap_frame_next },
    { "wK", &screen_swap_frame_prev },
    { "R",  &handle_force_redraw }
};

static void normal_mode(void);
static void search_mode(void);
static void print_status(void);

void input_handle_key(char ch) {
    unsigned int len = (Key_Buffer == NULL) ? 0 : strlen(Key_Buffer);

    /* If no more room then alert the user */
    if (len+1 >= Key_Buffer_Size) {
        if (Key_Buffer_Size == 0)
            Key_Buffer_Size = 8;
        else
            Key_Buffer_Size *= 2;
        Key_Buffer = realloc(Key_Buffer, Key_Buffer_Size);
        Key_Buffer[len] = '\0';
    }

    /* Escape key puts us back into normal mode */
    if (ch == ESC) {
        Key_Buffer[0] = '\0';
        state_set_mode(MODE_NORMAL);
    }
    /* If we got a valid key then process it */
    else if((char)ERR != ch) {
        /* Put the key in the buffer */
        len++;
        Key_Buffer[len-1] = ch;
        Key_Buffer[len]   = '\0';

        /* Execute the proper mode handler */
        switch(state_get_mode()) {
            case MODE_NORMAL: normal_mode(); break;
            case MODE_SEARCH: search_mode(); break;
            default: break;
        }
    }

    /* Update the status line */
    print_status();
}

static void normal_mode(void) {
    static const size_t num_entries = (sizeof(Default_Bindings) / sizeof(binding_t));
    bool any_matches = false;
    size_t len = strlen(Key_Buffer);

    /* Loop over the bindings */
    for(size_t i = 0; i < num_entries; i++) {
        binding_t binding = Default_Bindings[i];
        char* seq = binding.sequence;

        /* check if the binding matches the buffer as entered so far */
        if (0 == strncmp(Key_Buffer, seq, len)) {
            any_matches = true;
            if(strlen(seq) == len) { /* an exact match. execute it */
                binding.callback();
                Key_Buffer[0] = '\0';
                break;
            }
        }
    }

    /* If we did not find a match and we don't have any possibility of
     * finding a longer match, then throw out the buffer and start over */
    if(!any_matches) {
        beep();
        flash();
        len = 0;
        Key_Buffer[0] = '\0';
    }
}

static void search_mode(void) {
    size_t len = strlen(Key_Buffer);
    char prev = Key_Buffer[len-1];
    if (prev == '\n') {
        handle_cd();
        Key_Buffer[0] = '\0';
        state_set_mode(MODE_NORMAL);
    } else {
        workdir_seek(state_get_focused_workdir(), Key_Buffer);
    }
    if(state_get_refresh_state() != REFRESH_COMPLETE) screen_update();
}

static void print_status(void) {
    int lines, cols;
    getmaxyx(stdscr, lines, cols);
    (void)cols;
    move(lines-1, 0);
    clrtoeol();
    switch(state_get_mode()) {
        case MODE_NORMAL:
            printw(Key_Buffer);
            break;

        case MODE_SEARCH:
            printw("/%s", Key_Buffer);
            break;

        default:
            break;
    }
}

