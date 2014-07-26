#include "input.h"
#include "state.h"
#include "workdir.h"
#include "screen.h"
#include <stdlib.h>
#include <string.h>

typedef void (*key_cb_t)(void);

typedef struct {
    char* sequence;
    key_cb_t callback;
} binding_t;

static void handle_aardvark(void) {
    state_set_aardvark_mode(!state_get_aardvark_mode());
    state_set_screen_dirty(true);
}

static void handle_quit(void) {
    state_set_running(false);
}

static void handle_next(void) {
    workdir_next(state_get_focused_frame()->workdir);
}

static void handle_prev(void) {
    workdir_prev(state_get_focused_frame()->workdir);
}

static void handle_cd(void) {
    workdir_cd(state_get_focused_frame()->workdir);
}

static void handle_scroll_to_top(void) {
	workdir_scroll_to_top(state_get_focused_frame()->workdir);
}
static void handle_scroll_to_bottom(void) {
	workdir_scroll_to_bot(state_get_focused_frame()->workdir);
}
static void handle_page_up(void){
	workdir_jump_up(state_get_focused_frame()->workdir);
}
static void handle_page_down(void){
	workdir_jump_down(state_get_focused_frame()->workdir);
}

static void search_mode(void){
}

static binding_t Default_Bindings[] = {
    { "a",  &handle_aardvark },
    { "q",  &handle_quit },
    { "j",  &handle_next },
    { "k",  &handle_prev },
    { "\n", &handle_cd },
    { "wn", &screen_open },
    { "wc", &screen_close },
    { "/",  &search_mode },
    { "gg", &handle_scroll_to_top },
    { "G",  &handle_scroll_to_bottom },
    { "U",  &handle_page_up },
    { "D",  &handle_page_down },
    //{ "wj", NULL },
    //{ "wk", NULL },
};

static char Key_Buffer[16] = {0};

void input_handle_key(char ch) {
    bool more_matches = false;
    bool match_found  = false;
    size_t num_entries = (sizeof(Default_Bindings) / sizeof(binding_t));
    int len = strlen(Key_Buffer);

    /* If no more room then reset the buffer */
    if (len+1 >= 16) {
        beep();
        flash();
        len = 0;
        Key_Buffer[0] = '\0';
    }

    /* If we got a valid key then process it */
    if(ERR != ch) {
        /* Put the key in the buffer */
        len++;
        Key_Buffer[len-1] = ch;
        Key_Buffer[len]   = '\0';

        /* Loop over the bindings */
        for(int i = 0; i < num_entries; i++) {
            binding_t binding = Default_Bindings[i];
            char* seq = binding.sequence;

            /* If the binding we're looking at matches a substring but has more chars
             * make note of it so we can wait for the next char */
            if((strlen(seq) > len) && (0 == strncmp(seq, Key_Buffer, len)))
            {
                more_matches = true;
            }
            /* If the current string matches exactly then execute it's handler */
            if (0 == strcmp(Key_Buffer, seq))
            {
                binding.callback();
                Key_Buffer[0] = '\0';
                match_found = true;
                break;
            }
        }

        /* If we did not find a match and we don't have any possibility of
         * finding a longer match, then throw out the buffer and start over */
        if(!match_found && !more_matches) {
            beep();
            flash();
            len = 0;
            Key_Buffer[0] = '\0';
        }
    }
}

