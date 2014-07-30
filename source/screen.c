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

/* TODO: add equiv. function to list */
static int get_index_of_node(list_node_t* node){
    int i = 0;
    list_node_t* edon = Frame_List->head;
    while(edon!=NULL && edon != node) { edon = edon->next; i++; }
    return (edon!=NULL ? i : -1 );
}

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
    if(state_get_refresh_state() == REFRESH_AARDVARK && state_get_aardvark_mode()){
        aardvark_draw();
    } else if(state_get_refresh_state() == REFRESH_CURR_WIN){
        screen_refresh_curr_frame();
    } else {
        endwin();
        screen_place_windows();
    }
    /* Refresh and mark complete */
    state_set_refresh_state(REFRESH_COMPLETE);
}

void screen_open(void) {
    list_push_back(Frame_List, frame_new());
    state_set_refresh_state(REFRESH_ALL_WINS);
}

static int get_focused_frame_index(void){
    return get_index_of_node(state_get_focused_node());
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
        }else{
            fprintf(stderr, "frame not found\n");
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

//TODO: this should be a function in list.h
list_node_t* find_prev_node(list_node_t* node){
    list_node_t* prev = (Frame_List->head == node) ? NULL : Frame_List->head;
    while(prev && prev->next != node) prev=prev->next;
    return prev;
}

void screen_focus_prev(void){
    list_node_t* prev = find_prev_node(state_get_focused_node());
    if(!prev) prev = Frame_List->tail;
    state_set_focused_node(prev);
    state_set_refresh_state(REFRESH_CURR_WIN);
}

void screen_focus_master(void){
    state_set_focused_node(Frame_List->head);
    state_set_refresh_state(REFRESH_CURR_WIN);
}

void screen_swap_with_master(void){
    //this almost works. may need to manually force refresh (R) after repositioning windows
    //unknown why screen_force_redraw at bottom is insufficient
    //TODO: this should be done by callign functions in list.h
    //but reqd functions do not exist yet
    list_node_t* focused = state_get_focused_node();
    list_node_t* master = Frame_List->head;
    list_node_t* prev = find_prev_node(focused);
    list_node_t* tmp = master->next;
    if(prev){ //if prev is null, implies focus is already master & should do nothing
        //put master in list
        if(prev!=master) prev->next = master;
        master->next = focused->next;
        //make focused new heaad
        if(focused != tmp) focused->next = tmp;
        else focused->next = master;
        Frame_List->head = focused;
        //fix tail if put master at end
        if(master->next == NULL) Frame_List->tail = master;
        state_set_refresh_state(REFRESH_ALL_WINS);
    }
}

