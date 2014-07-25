#include "input.h"
#include "state.h"
#include "workdir.h"
#include "screen.h"
#include <stdlib.h>

static vec_t* States;
static vec_t* Curr_State;

typedef void (*key_cb_t)(void);

typedef struct {
    char* sequence;
    key_cb_t callback;
} binding_t;

static void handle_aardvark(void) {
    state_set_aardvark_mode(!state_get_aardvark_mode());
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

binding_t Default_Bindings[] = {
    { "a",  &handle_aardvark },
    { "q",  &handle_quit },
    { "j",  &handle_next },
    { "k",  &handle_prev },
    { "\n", &handle_cd },
    { "wn", &screen_open },
    { "wc", &screen_open },
};

void input_init(void) {
    size_t num_entries = (sizeof(Default_Bindings) / sizeof(binding_t));
    for(int i = 0; i < num_entries; i++) {
        input_add_binding(Default_Bindings[i].sequence, Default_Bindings[i].callback);
    }
    exit(0);
}

void input_handle_key(char ch) {
    ///* Assume screen is dirty by default */
    //bool is_screen_dirty = true;
    //switch(ch){
    //    case 'a': state_set_aardvark_mode(!state_get_aardvark_mode());
    //              break;
    //    case 'q': state_set_running(false);
    //              break;
    //    case 'j': workdir_next(state_get_focused_frame()->workdir); break;
    //    case 'k': workdir_prev(state_get_focused_frame()->workdir); break;
    //    case 'e': workdir_cd(state_get_focused_frame()->workdir); break;
    //    case 'n': screen_open();
    //              break;
    //    case 'c': screen_close();
    //              break;
    //    default:  is_screen_dirty = false;
    //              break;
    //}
    ///* Update the screen if we need to */
    //state_set_screen_dirty(state_get_screen_dirty() || is_screen_dirty);
}

void input_add_binding(char* seq, key_cb_t p_cb_fn) {
    vec_t* p_state = States;
    size_t len = strlen(seq);
    for(int i = 0; i < len; i++) {
        key_t* p_key = input_lookup_or_add(p_state, seq[i]);
    }
}

