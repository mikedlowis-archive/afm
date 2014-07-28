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
void frame_page_up(Frame_T* p_frame);
void frame_page_down(Frame_T* p_frame);
void frame_draw_files(Frame_T* frame); //TODO: name pointers consistently
void frame_set_highlighting(Frame_T* frame, bool highlight, bool refresh_win);

#endif /* FRAME_H */

