/**
  @file screen.h
  @brief TODO: Describe this file
  $Revision$
  $HeadURL$
  */
#ifndef SCREEN_H
#define SCREEN_H

void screen_init(void);
void screen_deinit(void);
void screen_update(void);
void screen_open(void);
void screen_close(void);
void screen_focus_next(void);
void screen_focus_prev(void);
void screen_focus_master(void);
void screen_swap_with_master(void);
void screen_force_redraw(void);

#endif /* SCREEN_H */
