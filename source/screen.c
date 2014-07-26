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
#include "state.h"
#include "aardvark.h"
#include "workdir.h"

static void screen_place_windows(void);
static frame_t* screen_frame_new(void);
static void screen_frame_free(void* p_frame);
void screen_frame_draw_files(frame_t* frame);

static list_t* Screen_List;

static frame_t* master_frame(void){
    return (frame_t*) Screen_List->head->contents;
}

void screen_init(void) {
    Screen_List = list_new();
    list_push_back(Screen_List, screen_frame_new());
    state_set_focused_frame(master_frame());
}

void screen_deinit(void) {
	fprintf(stderr, "deinitializing screen\n");
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
    state_set_screen_dirty(true);
}

void screen_close(void) {
    int num_frames = list_size(Screen_List);
    if(num_frames > 1){
        list_delete(Screen_List, 0);
        state_set_focused_frame(master_frame());
    }
    state_set_screen_dirty(true);
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
    screen_frame_draw_files(p_frame);
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
        screen_frame_draw_files(p_frame);
        wmove(p_frame->p_win, 1, 1);
        box(p_frame->p_win, 0 , 0);
        wrefresh(p_frame->p_win);
        /* Get the next one */
        id++;
        pos += height;
        p_node = p_node->next;
    }
}

//get the curent directory and copy it into a ref-counted memory block
//return a pointer to the new block
char* pwd(){
	char* dir = getcwd(NULL, 0);
	char* rid = mem_allocate(sizeof(char)*(1+strlen(dir)), NULL);
	strcpy(rid, dir);
	free(dir);
	return rid;
}

static frame_t* screen_frame_new(void) {
    frame_t* p_frame = (frame_t*)mem_allocate(sizeof(frame_t),&screen_frame_free);
    p_frame->p_win = newwin(1, 1, 0, 0);
    bool first_window = !state_get_focused_frame();
    char* path = first_window ? pwd() : state_get_focused_frame()->workdir->path;
    p_frame->workdir = workdir_new(path);
    if(first_window) mem_release(path);
    return p_frame;
}

static void screen_frame_free(void* p_frame_ptr) {
	fprintf(stderr, "freeing frame\n");
    frame_t* p_frame = (frame_t*)p_frame_ptr;
    wclear(p_frame->p_win);
    wborder(p_frame->p_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(p_frame->p_win);
    delwin(p_frame->p_win);
    if(p_frame->workdir) mem_release(p_frame->workdir);
}

void screen_frame_draw_files(frame_t* frame){
    int i = frame->workdir->top_index;
    int rows, cols;
    getmaxyx(frame->p_win, rows, cols);
    //draw path
    wattron(frame->p_win, A_UNDERLINE);
    mvwaddnstr(frame->p_win, 1, 1, frame->workdir->path, cols-2);
    wattroff(frame->p_win, A_UNDERLINE);
    //list files
    while (i < vec_size(frame->workdir->vfiles)){
        if(frame == state_get_focused_frame() && i == frame->workdir->idx){
            wattron(frame->p_win, A_STANDOUT);
            wattron(frame->p_win, A_BOLD);
        }
        mvwaddnstr(frame->p_win, FrameTopBuffer+i-frame->workdir->top_index, 1, vec_at(frame->workdir->vfiles, i), cols-2);
        if(frame == state_get_focused_frame() && i == frame->workdir->idx){
            wattroff(frame->p_win, A_STANDOUT);
            wattroff(frame->p_win, A_BOLD);
        }
        i++;
        if((FrameTopBuffer+i-frame->workdir->top_index+FrameBotBuffer) > rows) break;
    }
}


