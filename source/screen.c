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
    /* Initialize ncurses */
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
    /* Initialize the frame list */
    Frame_List = list_new();
    list_push_back(Frame_List, frame_new());
    state_set_focused_node(Frame_List->head);
}

void screen_deinit(void) {
    /* dump the frame list */
    mem_release(Frame_List);
    /* tear down ncurses */
    clear();
    refresh();
    endwin();
}

void screen_update(void) {
    /* Clear screen and update LINES and COLS */
    if(state_get_refresh_state() == REFRESH_ALL_WINS){
        endwin();
        screen_place_windows();
    } else {
        screen_refresh_curr_frame();
    }
    if(state_get_aardvark_mode()) aardvark_draw();
    /* Refresh and mark complete */
    state_set_refresh_state(REFRESH_COMPLETE);
}

void screen_open(void) {
    list_push_back(Frame_List, frame_new());
    state_set_refresh_state(REFRESH_ALL_WINS);
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
        if(i >= 0){ /* negative if node not found */
            list_node_t* doomed_node = state_get_focused_node();
            mem_retain(doomed_node);
            list_node_t* new_focus = doomed_node->next;
            /* TODO: add function to list that allows removing with node pointer instead of just index */
            list_delete(Frame_List, i);
            // new_focus will be null if rm-d tail: set it to new tail
            if(new_focus == NULL) new_focus = Frame_List->tail;
            state_set_focused_node(new_focus);
            state_set_refresh_state(REFRESH_ALL_WINS);
            mem_release(doomed_node);
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
    frame_move(p_frame, 0, 0);
    frame_resize(p_frame, lines, (num_frames > 1) ? cols/2 : cols);
    frame_draw(p_frame);

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
        frame_move(p_frame, pos, cols/2);
        frame_resize(p_frame, height, cols/2);
        frame_draw(p_frame);
        /* Get the next one */
        id++;
        pos += height;
        p_node = p_node->next;
    }
}

static void screen_refresh_curr_frame(void) {
    Frame_T* p_frame = state_get_focused_frame();
    frame_draw(p_frame);
}

void screen_focus_next(void){
    list_node_t* focused = state_get_focused_node();
    state_set_focused_node(focused->next ? focused->next : Frame_List->head);
    state_set_refresh_state(REFRESH_CURR_WIN);
}

void screen_focus_prev(void){
    int i = get_focused_frame_index();
    if(i >= 0){
        list_node_t* prev = (i == 0) ? Frame_List->tail : list_at(Frame_List, i-1);
        if(prev) state_set_focused_node(prev);
        state_set_refresh_state(REFRESH_CURR_WIN);
    }
}

void screen_focus_master(void){
    state_set_focused_node(Frame_List->head);
    state_set_refresh_state(REFRESH_CURR_WIN);
}

