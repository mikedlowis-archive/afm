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

#define STATUS_LINE_MARGIN (1u)

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
    if(state_get_refresh_state() == REFRESH_CURR_WIN){
        screen_refresh_curr_frame();
    } else if (state_get_refresh_state() == REFRESH_ALL_WINS) {
        endwin();
        screen_place_windows();
    }
    if(state_get_aardvark_mode() && state_get_refresh_state() != REFRESH_COMPLETE)
        aardvark_draw();
    /* Refresh and mark complete */
    state_set_refresh_state(REFRESH_COMPLETE);
}

void screen_open(void) {
    list_push_back(Frame_List, frame_new());
    state_set_refresh_state(REFRESH_ALL_WINS);
}

void screen_close(void) {
    if (Frame_List->head != Frame_List->tail) {
        list_node_t* doomed_node = state_get_focused_node();
        list_node_t* new_focus = doomed_node->next;
        if(new_focus == NULL) new_focus = Frame_List->tail;
        mem_retain(doomed_node);
        list_delete_node(Frame_List, doomed_node);
        state_set_focused_node(new_focus);
        state_set_refresh_state(REFRESH_ALL_WINS);
        mem_release(doomed_node);
    } /* else only one window open. do nothing */
}

static void screen_place_windows(void) {
    Frame_T* p_frame;
    int id, pos, lines, cols;
    int num_sub_frames = list_size(Frame_List) - 1;
    list_node_t* p_node;
    getmaxyx(stdscr, lines, cols);
    lines-=STATUS_LINE_MARGIN;
    /* Print the master frame */
    p_frame = list_at(Frame_List,0)->contents;
    frame_move(p_frame, 0, 0);
    frame_resize(p_frame, lines, (num_sub_frames > 0) ? cols/2 : cols);
    frame_draw(p_frame);

    /* Print any other frames we might have */
    p_node = list_at(Frame_List,1);
    pos = 0;
    id = 1;
    while(p_node != NULL) {
        /* Get the frame and it's properties */
        int remain = (lines % num_sub_frames);
        int height = (lines / num_sub_frames) + (id <= remain ? 1 : 0);
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
    list_node_t* prev = list_prev(Frame_List, state_get_focused_node());
    if(!prev) prev = Frame_List->tail;
    state_set_focused_node(prev);
    state_set_refresh_state(REFRESH_CURR_WIN);
}

void screen_focus_master(void){
    state_set_focused_node(Frame_List->head);
    state_set_refresh_state(REFRESH_CURR_WIN);
}

//for when force refresh (R) fixes the screen,
//but setting screen REFRESH_ALL_WINS doesnt
//necessary when moving frames around
static void stoopid_redraw(Frame_T* a, Frame_T* b){
    frame_resize(a, 1, 1);
    frame_move(a, 0, 0);
    frame_resize(b, 1, 1);
    frame_move(b, 1, 1);
    state_set_refresh_state(REFRESH_ALL_WINS);
}

void screen_swap_with_master(void){
    list_node_t* focused = state_get_focused_node();
    list_node_t* master = Frame_List->head;
    if(focused != master){
        Frame_T* fmast = (Frame_T*)master->contents;
        Frame_T* ffoc = (Frame_T*)focused->contents;
        mem_retain(fmast);
        mem_retain(ffoc);
        list_delete_node(Frame_List, master);
        list_insert_after(Frame_List, focused, fmast);
        list_delete_node(Frame_List, focused);
        list_insert_after(Frame_List, NULL, ffoc);
        // reset focused window (since old focused destroyed)
        state_set_focused_node(Frame_List->head);
        // Resize and move os they don't overlap when we place them.
        stoopid_redraw(fmast, ffoc);
    }
}

void screen_swap_frame_next(void){
    if(Frame_List->head != Frame_List->tail){
        list_node_t* focused = state_get_focused_node();
        list_node_t* next = focused->next;
        list_node_t* new_node = NULL;
        Frame_T* ffoc = (Frame_T*)focused->contents;
        mem_retain(ffoc);
        state_set_focused_node(NULL);
        list_delete_node(Frame_List, focused);
        new_node = list_insert_after(Frame_List, next, ffoc);
        state_set_focused_node(new_node);
        stoopid_redraw(ffoc, (Frame_T*) ((NULL == next) ? Frame_List->tail->contents : next->contents));
    }
}

void screen_swap_frame_prev(void){
    if(Frame_List->head != Frame_List->tail){
        list_node_t* focused = state_get_focused_node();
        if(focused != Frame_List->head){
            list_node_t* prev = list_prev(Frame_List, focused);
            Frame_T* ffoc = (Frame_T*)focused->contents;
            Frame_T* fpre = (Frame_T*)prev->contents;
            mem_retain(fpre);
            list_delete_node(Frame_List, prev);
            list_insert_after(Frame_List, focused, fpre);
            stoopid_redraw(ffoc, fpre);
        }else{
            list_node_t* prev = Frame_List->tail;
            Frame_T* ffoc = (Frame_T*)focused->contents;
            Frame_T* fpre = (Frame_T*)prev->contents;
            list_node_t* new_node = NULL;
            mem_retain(ffoc);
            mem_retain(fpre);
            state_set_focused_node(NULL);
            list_delete_node(Frame_List, focused);
            list_delete_node(Frame_List, prev);
            list_insert_after(Frame_List, NULL, fpre);
            new_node = list_insert_after(Frame_List, Frame_List->tail, ffoc);
            state_set_focused_node(new_node);
            stoopid_redraw(ffoc, fpre);
        }
    }
}

