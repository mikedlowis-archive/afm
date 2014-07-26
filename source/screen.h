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

//number of lines to leave before/after dir contents in frame
static int FrameTopBuffer = 2;
static int FrameBotBuffer = 2;

typedef struct {
    WINDOW* p_win;
    WorkDir_T* workdir;
    int top_index;
} frame_t;

enum ColorPairs {
	DIRECTORY = 1
};

#endif /* SCREEN_H */
