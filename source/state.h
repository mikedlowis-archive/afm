/**
  @file state.h
  @brief TODO: Describe this file
  $Revision$
  $HeadURL$
  */
#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include "screen.h"

typedef enum{ MODE_NORMAL, MODE_SEARCH } MODE;

bool state_get_running(void);
void state_set_running(bool val);
bool state_get_screen_dirty(void);
void state_set_screen_dirty(bool val);
bool state_get_screen_resized(void);
void state_set_screen_resized(bool val);
bool state_get_aardvark_mode(void);
void state_set_aardvark_mode(bool val);
frame_t* state_get_focused_frame(void);
void state_set_focused_frame(frame_t* p_frame);

#endif /* STATE_H */
