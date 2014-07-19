/**
  @file state.h
  @brief TODO: Describe this file
  $Revision$
  $HeadURL$
  */
#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

bool state_get_running(void);
void state_set_running(bool val);
bool state_get_screen_dirty(void);
void state_set_screen_dirty(bool val);
bool state_get_screen_resized(void);
void state_set_screen_resized(bool val);
bool state_get_aardvark_mode(void);
void state_set_aardvark_mode(bool val);
int state_get_focused_frame(void);
void state_set_focused_frame(int id);

#endif /* STATE_H */
