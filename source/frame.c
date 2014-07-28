/* external libraries */
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>

/* internal libraries */
#include "mem.h"
#include "vec.h"

/* internal headers */
#include "frame.h"
#include "workdir.h"
#include "state.h"

static int FrameTopBuffer = 2;
static int FrameBotBuffer = 2;

static void frame_free(void* p_frame);

//get the curent directory and copy it into a ref-counted memory block
//return a pointer to the new block
char* pwd(){
    char* dir = getcwd(NULL, 0);
    char* rid = mem_allocate(sizeof(char)*(1+strlen(dir)), NULL);
    strcpy(rid, dir);
    free(dir);
    return rid;
}

Frame_T* frame_new(void) {
    Frame_T* p_frame = (Frame_T*)mem_allocate(sizeof(Frame_T),&frame_free);
    p_frame->p_win = newwin(1, 1, 0, 0);
    p_frame->top_index = 0;
    bool first_window = !state_get_focused_frame();
    char* path = first_window ? pwd() : state_get_focused_workdir()->path;
    p_frame->workdir = workdir_new(path);
    if(first_window) mem_release(path);
    return p_frame;
}

static void frame_free(void* p_frame_ptr) {
    Frame_T* p_frame = (Frame_T*)p_frame_ptr;
    wclear(p_frame->p_win);
    wborder(p_frame->p_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(p_frame->p_win);
    delwin(p_frame->p_win);
    if(p_frame->workdir) mem_release(p_frame->workdir);
}

static int count_double_lines(Frame_T* p_frame){
    int count = 0;
    for(int i = p_frame->top_index; i <= p_frame->workdir->idx; i++)
        if (((File_T*)vec_at(p_frame->workdir->vfiles, i))->expanded) count++;
    return count;
}

static void frame_scroll(Frame_T* p_frame){
    int rows,cols;
    getmaxyx(p_frame->p_win, rows, cols);
    (void) cols;
    if(p_frame->workdir->idx < p_frame->top_index){
        p_frame->top_index = p_frame->workdir->idx;
    }else{
        int doublelines = count_double_lines(p_frame);
        if (p_frame->top_index < doublelines+p_frame->workdir->idx-(rows-FrameTopBuffer-FrameBotBuffer))
            p_frame->top_index = doublelines+p_frame->workdir->idx-(rows-FrameTopBuffer-FrameBotBuffer);
    }
}

int realrows(Frame_T* p_frame){
    int rows, cols;
    getmaxyx(p_frame->p_win, rows, cols);
    (void) cols;
    return rows - FrameTopBuffer - FrameBotBuffer;
}

void frame_page_up(Frame_T* p_frame){
    workdir_set_idx(p_frame->workdir, p_frame->workdir->idx - realrows(p_frame));
}

void frame_page_down(Frame_T* p_frame){
    workdir_set_idx(p_frame->workdir, p_frame->workdir->idx+realrows(p_frame));
}

void frame_draw_files(Frame_T* frame){
    int file_i, frame_i = FrameTopBuffer;
    int rows, cols;
    getmaxyx(frame->p_win, rows, cols);
    frame_scroll(frame);
    file_i = frame->top_index;
    //draw path
    wattron(frame->p_win, A_UNDERLINE);
    mvwaddnstr(frame->p_win, 1, 1, frame->workdir->path, cols-2);
    wattroff(frame->p_win, A_UNDERLINE);
    //list files
    while (file_i < vec_size(frame->workdir->vfiles)){
        File_T* file = (File_T*)vec_at(frame->workdir->vfiles, file_i);
        bool dir = is_dir(file->path);
        if(frame == state_get_focused_frame() && file_i == frame->workdir->idx){
            wattron(frame->p_win, A_STANDOUT | A_BOLD);
        }
        if(dir) wattron(frame->p_win, COLOR_PAIR(DIRECTORY));
        mvwaddnstr(frame->p_win, frame_i, 1, file->name, cols-2);
        frame_i++;
        if(file->expanded){
            mvwprintw(frame->p_win, frame_i, 1, "    owned by user id %d, group id %d", file->uid, file->gid);
            frame_i++;
        }
        wstandend(frame->p_win);
        file_i++;
        if((frame_i+FrameBotBuffer) > rows) break;
    }
}


void frame_set_highlighting(Frame_T* frame, bool highlight, bool refresh_win){
	if(frame){
		int line = FrameTopBuffer + frame->workdir->idx - frame->top_index;
		attr_t newattr= highlight ? (A_STANDOUT|A_BOLD) : A_NORMAL;
		File_T* file = (File_T*) vec_at(frame->workdir->vfiles, frame->workdir->idx);
		short color = (is_dir(file->path) ? DIRECTORY : 0);
		mvwchgat(frame->p_win, line, 0, -1, newattr, color, NULL);
		if(file->expanded) mvwchgat(frame->p_win, line+1, color, -1, newattr, 0, NULL);
		if(refresh_win) wrefresh(frame->p_win);
	}
}

