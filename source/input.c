#include "input.h"
#include "state.h"
#include "workdir.h"
#include "screen.h"

void input_handle_key(char ch) {
    /* Assume screen is dirty by default */
    bool is_screen_dirty = true;
    switch(ch){
        case 'a': state_set_aardvark_mode(!state_get_aardvark_mode());
                  break;
        case 'q': state_set_running(false);
                  break;
        case 'j': workdir_next();
                  break;
        case 'k': workdir_prev();
                  break;
        case 'e': workdir_cd();
                  break;
        case 'n': screen_open();
                  break;
        case 'c': screen_close();
                  break;
        default:  is_screen_dirty = false;
                  break;
    }
    /* Update the screen if we need to */
    state_set_screen_dirty(state_get_screen_dirty() || is_screen_dirty);
}
