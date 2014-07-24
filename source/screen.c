#include "screen.h"
#include "state.h"
#include "aardvark.h"
#include "workdir.h"
#include <ncurses.h>
#include "list.h"
#include "mem.h"

typedef struct {
    WINDOW* p_win;
} frame_t;

static void screen_place_windows(void);
static frame_t* screen_frame_new(void);
static void screen_frame_free(void* p_frame);

static list_t* Screen_List;

void screen_init(void) {
    Screen_List = list_new();
    list_push_back(Screen_List, screen_frame_new());
}

void screen_deinit(void) {
    mem_release(Screen_List);
}

void screen_update(void) {
    /* Clear screen and update LINES and COLS */
    if(state_get_screen_resized()){
        endwin();
        state_set_screen_resized(false);
    }
    clear();
    refresh();
    screen_place_windows();
    if(state_get_aardvark_mode()) aardvark_draw();
    /* Refresh and mark complete */
    state_set_screen_dirty(false);
}

void screen_open(void) {
    list_push_back(Screen_List, screen_frame_new());
}

void screen_close(void) {
    int num_frames = list_size(Screen_List);
    if(num_frames > 1)
        list_delete(Screen_List, 0);
}

static void screen_place_windows(void) {
    frame_t* p_frame;
    int id, pos, lines, cols;
    int num_frames = list_size(Screen_List);
    list_node_t* p_node;
    getmaxyx(stdscr, lines, cols);

    /* Print the master frame */
    p_frame = list_at(Screen_List,0)->contents;
    mvwin(p_frame->p_win, 0, 0);
    wresize(p_frame->p_win, lines, (num_frames > 1) ? cols/2 : cols);
    wclear(p_frame->p_win);
    box(p_frame->p_win, 0 , 0);
    wrefresh(p_frame->p_win);

    /* Print any other frames we might have */
    p_node = list_at(Screen_List,1);
    pos = 0;
    id = 1;
    while(p_node != NULL) {
        /* Get the frame and it's properties */
        int remain = (lines % (num_frames-1));
        int height = (lines / (num_frames-1)) + (id <= remain ? 1 : 0);
        p_frame = p_node->contents;
        /* Place the frame */
        mvwin(p_frame->p_win, pos, cols/2);
        wresize(p_frame->p_win, height, cols/2);
        wclear(p_frame->p_win);
        wmove(p_frame->p_win, 1, 1);
        box(p_frame->p_win, 0 , 0);
        wrefresh(p_frame->p_win);
        /* Get the next one */
        id++;
        pos += height;
        p_node = p_node->next;
    }
}

static frame_t* screen_frame_new(void) {
    frame_t* p_frame = (frame_t*)mem_allocate(sizeof(frame_t),&screen_frame_free);
    p_frame->p_win = newwin(1, 1, 0, 0);
    return p_frame;
}

static void screen_frame_free(void* p_frame_ptr) {
    frame_t* p_frame = (frame_t*)p_frame_ptr;
    wborder(p_frame->p_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(p_frame->p_win);
    delwin(p_frame->p_win);
}
