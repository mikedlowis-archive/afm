#ifndef FRAME_H
#define FRAME_H

#include <ncurses.h>
#include "workdir.h"

typedef struct {
    WINDOW* p_win;
    WorkDir_T* workdir;
    int top_index;
} Frame_T;


enum ColorPairs {
    DIRECTORY = 1
};

Frame_T* frame_new(void);
void frame_resize(Frame_T* p_frame, int lines, int cols);
void frame_move(Frame_T* p_frame, int line, int col);
void frame_draw(Frame_T* p_frame);

void frame_page_up(Frame_T* p_frame);
void frame_page_down(Frame_T* p_frame);
void frame_draw_files(Frame_T* frame); //TODO: name pointers consistently
void frame_set_highlighting(Frame_T* frame, bool highlight, bool refresh_win);
void frame_reload(Frame_T* frame);

#endif /* FRAME_H */

