/* external libraries */
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* internal libraries */
#include "vec.h"
#include "list.h"
#include "mem.h"

/* internal headers */
#include "screen.h"
#include "frame.h"
#include "state.h"
#include "aardvark.h"

static void screen_place_windows(void);
static void screen_refresh_curr_frame(void);

static list_t* Frame_List;

void screen_init(void) {
    Frame_List = list_new();
    list_push_back(Frame_List, frame_new());
    state_set_focused_node(Frame_List->head);
}

void screen_deinit(void) {
    mem_release(Frame_List);
}

void screen_update(void) {
    /* Clear screen and update LINES and COLS */
    if(state_get_screen_resized()){
        endwin();
        screen_place_windows();
        state_set_screen_resized(false);
    } else {
        screen_refresh_curr_frame();
    }
    if(state_get_aardvark_mode()) aardvark_draw();
    /* Refresh and mark complete */
    state_set_screen_dirty(false);
}

void screen_open(void) {
    list_push_back(Frame_List, frame_new());
    state_set_screen_dirty(true);
    state_set_screen_resized(true);
}

/* TODO: add equiv. function to list */
static int get_focused_frame_index(void){
    int i = 0;
    list_node_t* n = Frame_List->head;
    while(n != state_get_focused_node() && n != Frame_List->tail){ n = n->next; i++; }
    if(n != state_get_focused_node()) i = -1;
    return i;
}

void screen_close(void) {
    if (Frame_List->head != Frame_List->tail) {
        int i = get_focused_frame_index();
        list_node_t* new_focus = state_get_focused_node()->next;
        if(i >= 0){ /* negative if node not found */
            /* TODO: add function to list that allows removing with node pointer instead of just index */
            list_delete(Frame_List, i);
            // new_focus will be null if rm-d tail: set it to new tail
            if(new_focus == NULL) new_focus = Frame_List->tail;
            state_set_focused_node(new_focus);
            state_set_screen_dirty(true);
            state_set_screen_resized(true);
        }
    }
}

static void screen_place_windows(void) {
    Frame_T* p_frame;
    int id, pos, lines, cols;
    int num_frames = list_size(Frame_List);
    list_node_t* p_node;
    getmaxyx(stdscr, lines, cols);

    /* Print the master frame */
    p_frame = list_at(Frame_List,0)->contents;
    mvwin(p_frame->p_win, 0, 0);
    wresize(p_frame->p_win, lines, (num_frames > 1) ? cols/2 : cols);
    wclear(p_frame->p_win);
    frame_draw_files(p_frame);
    box(p_frame->p_win, 0 , 0);
    wrefresh(p_frame->p_win);

    /* Print any other frames we might have */
    p_node = list_at(Frame_List,1);
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
        frame_draw_files(p_frame);
        wmove(p_frame->p_win, 1, 1);
        box(p_frame->p_win, 0 , 0);
        wrefresh(p_frame->p_win);
        /* Get the next one */
        id++;
        pos += height;
        p_node = p_node->next;
    }
}

static void screen_refresh_curr_frame(void) {
    Frame_T* p_frame = state_get_focused_frame();
    wclear(p_frame->p_win);
    frame_draw_files(p_frame);
    box(p_frame->p_win, 0 , 0);
    wrefresh(p_frame->p_win);
}

void screen_focus_next(void){
    list_node_t* focused = state_get_focused_node();
    if(focused->next != NULL){
        state_set_focused_node(focused->next);
        state_set_screen_dirty(true);
    }
}

void screen_focus_prev(void){
    int i = get_focused_frame_index();
    if(i > 0){
        list_node_t* prev = list_at(Frame_List, i-1);
        if(prev) state_set_focused_node(prev);
        state_set_screen_dirty(true);
    }
}

void screen_focus_master(void){
    state_set_focused_node(Frame_List->head);
    state_set_screen_dirty(true);
}

