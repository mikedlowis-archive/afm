/**
  @file screen.h
  @brief TODO: Describe this file
  $Revision$
  $HeadURL$
  */
#ifndef SCREEN_H
#define SCREEN_H

#include <ncurses.h>
#include "workdir.h"

void screen_init(void);
void screen_deinit(void);
void screen_update(void);
void screen_open(void);
void screen_close(void);

typedef struct {
    WINDOW* p_win;
    WorkDir_T* workdir;
    int top_index;
} frame_t;

enum ColorPairs {
    DIRECTORY = 1
};

void screen_frame_page_up(frame_t* p_frame);
void screen_frame_page_down(frame_t* p_frame);

#endif /* SCREEN_H */
