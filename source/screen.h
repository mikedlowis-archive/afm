/**
  @file screen.h
  @brief TODO: Describe this file
  $Revision$
  $HeadURL$
  */
#ifndef SCREEN_H
#define SCREEN_H

void screen_init(void);
void screen_update(void);
void screen_open(void);
void screen_close(void);

//number of lines to leave before/after dir contents in frame
static int FrameTopBuffer = 2;
static int FrameBotBuffer = 2;

#endif /* SCREEN_H */
