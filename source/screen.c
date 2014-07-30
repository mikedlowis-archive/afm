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

static bool check_frame_list(void){
	list_node_t* node = Frame_List->head;
	int i=0;
	while(node!=NULL){
		int j = get_index_of_node(node);
		if(i != j){
			fprintf(stderr, "problem! node %d came back as %d\n", i, j);
			return false;
		}
		i++;
		node = node->next;
	}
	return true;
}

void screen_init(void) {
    Frame_List = list_new();
    list_push_back(Frame_List, frame_new());
    state_set_focused_node(Frame_List->head);
    if(!check_frame_list()) fprintf(stderr, "something wak from teh start\n");
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
    if(!check_frame_list()) fprintf(stderr, "bad list before opening frame\n");
    list_push_back(Frame_List, frame_new());
    state_set_screen_dirty(true);
    state_set_screen_resized(true);
    if(!check_frame_list()) fprintf(stderr, "bad list after opening frame\n");
}

static int get_focused_frame_index(void){
	return get_index_of_node(state_get_focused_node());
	/*
    int i = 0;
    list_node_t* n = Frame_List->head;
    while(n != state_get_focused_node() && n != Frame_List->tail){ n = n->next; i++; }
    if(n != state_get_focused_node()) i = -1;
    return i;
    */
}

void screen_close(void) {
    if (Frame_List->head != Frame_List->tail) {
        int i = get_focused_frame_index();
        if(!check_frame_list()) fprintf(stderr, "bad list before closing frame\n");
        if(i >= 0){ /* negative if node not found */
            list_node_t* doomed_node = state_get_focused_node();
            mem_retain(doomed_node);
            list_node_t* new_focus = doomed_node->next;
            /* TODO: add function to list that allows removing with node pointer instead of just index */
            list_delete(Frame_List, i);
            // new_focus will be null if rm-d tail: set it to new tail
            if(new_focus == NULL) new_focus = Frame_List->tail;
            state_set_focused_node(new_focus);
            state_set_screen_dirty(true);
            state_set_screen_resized(true);
            mem_release(doomed_node);
        }else{
			fprintf(stderr, "frame not found\n");
		}
		if(!check_frame_list()) fprintf(stderr, "bad list after closing frame\n");
    }
}

static void screen_place_windows(void) {
    Frame_T* p_frame;
    int id, pos, lines, cols;
    int num_frames = list_size(Frame_List);
    list_node_t* p_node;
    getmaxyx(stdscr, lines, cols);
	if(!check_frame_list()) fprintf(stderr, "bad list when placing frames\n");
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
    state_set_focused_node(focused->next ? focused->next : Frame_List->head);
    state_set_screen_dirty(true);
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
	state_set_screen_dirty(true);
	/*
    int i = get_focused_frame_index();
    if(i >= 0){
        list_node_t* prev = (i == 0) ? Frame_List->tail : list_at(Frame_List, i-1);
        if(prev) state_set_focused_node(prev);
        state_set_screen_dirty(true);
    }
    */
}

void screen_focus_master(void){
    state_set_focused_node(Frame_List->head);
    state_set_screen_dirty(true);
}

void screen_swap_with_master(void){
	list_node_t* focused = state_get_focused_node();
	list_node_t* master = Frame_List->head;
	list_node_t* prev = find_prev_node(focused);
	list_node_t* tmp = master->next;
	if(!check_frame_list()) fprintf(stderr, "starting with a bad list\n");
	if(prev){ //if prev is null, implies focus is already master & should do nothing
		//put master in list
		if(prev!=master) prev->next = master;
		master->next = focused->next;
		//make focused new heaad
		if(focused != tmp) focused->next = tmp;
		else focused->next = master;
		Frame_List->head = focused;
		screen_force_redraw();
	}
	if(!check_frame_list()) fprintf(stderr, "ending with a bad list\n");
}

void screen_force_redraw(void){
	state_set_screen_dirty(true);
	state_set_screen_resized(true);
}

