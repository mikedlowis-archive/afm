#include "screen.h"
#include "state.h"
#include "aardvark.h"
#include "workdir.h"
#include <ncurses.h>
#include <unistd.h>
#include "vec.h"
#include "mem.h"

typedef struct {
    WINDOW* p_win;
    WorkDir_T* workdir;
} frame_t;

static void screen_place_windows(void);
static frame_t* screen_frame_new(void);
static void screen_frame_free(void* p_frame);
void screen_frame_draw_files(frame_t* frame);

static frame_t* Master;
static vec_t* Screen_List;

void screen_init(void) {
    Master = screen_frame_new();
    Screen_List = vec_new(0);
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
    vec_push_back(Screen_List, screen_frame_new());
}

void screen_close(void) {
    vec_erase(Screen_List, 0, 0);
}

static void screen_place_windows(void) {
    frame_t* p_frame;
    int i, lines, cols;
    getmaxyx(stdscr, lines, cols);

    /* Print the master frame */
    p_frame = Master;
    mvwin(p_frame->p_win, 0, 0);
    wresize(p_frame->p_win, lines, (vec_size(Screen_List) > 0) ? cols/2 : cols);
    wclear(p_frame->p_win);
    screen_frame_draw_files(p_frame);
    box(p_frame->p_win, 0 , 0);
    wrefresh(p_frame->p_win);

    /* Print any other frames we might have */
    int pos = 0;
    for(i = 0; i < vec_size(Screen_List); i++) {
        int remain = (lines % vec_size(Screen_List));
        int height = (lines / vec_size(Screen_List)) + (i < remain ? 1 : 0);
        p_frame = (frame_t*)vec_at(Screen_List, i);
        mvwin(p_frame->p_win, pos, cols/2);
        wresize(p_frame->p_win, height, cols/2);
        wclear(p_frame->p_win);
		screen_frame_draw_files(p_frame);
        wmove(p_frame->p_win, 1, 1);
        wprintw(p_frame->p_win, "(%d, %d)", i*height, cols/2);
        box(p_frame->p_win, 0 , 0);
        wrefresh(p_frame->p_win);
        pos += height;
    }
}

static frame_t* screen_frame_new(void) {
    frame_t* p_frame = (frame_t*)mem_allocate(sizeof(frame_t),&screen_frame_free);
    p_frame->p_win = newwin(1, 1, 0, 0);
    //TODO use current focused window's path
    p_frame->workdir = workdir_new(get_current_dir_name());
    return p_frame;
}

static void screen_frame_free(void* p_frame_ptr) {
    frame_t* p_frame = (frame_t*)p_frame_ptr;
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
		if(i == frame->workdir->idx){
			wattron(frame->p_win, A_STANDOUT);
			wattron(frame->p_win, A_BOLD);
		}
        mvwaddnstr(frame->p_win, FrameTopBuffer+i-frame->workdir->top_index, 1, vec_at(frame->workdir->vfiles, i), cols-2);
		if(i == frame->workdir->idx){
			wattroff(frame->p_win, A_STANDOUT);
			wattroff(frame->p_win, A_BOLD);
		}
		i++;
        if((FrameTopBuffer+i-frame->workdir->top_index+FrameBotBuffer) > rows) break;
	}
}


