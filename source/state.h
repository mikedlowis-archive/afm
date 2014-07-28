/**
  @file state.h
  @brief TODO: Describe this file
  $Revision$
  $HeadURL$
  */
#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include "list.h"

#include "frame.h"
#include "workdir.h"


typedef enum{ MODE_NORMAL, MODE_SEARCH } Mode_T;

bool state_get_running(void);
void state_set_running(bool val);
bool state_get_screen_dirty(void);
void state_set_screen_dirty(bool val);
bool state_get_screen_resized(void);
void state_set_screen_resized(bool val);
bool state_get_aardvark_mode(void);
void state_set_aardvark_mode(bool val);
void state_set_focused_node(list_node_t* p_node);
list_node_t* state_get_focused_node(void);
Frame_T* state_get_focused_frame(void);
WorkDir_T* state_get_focused_workdir(void);
Mode_T state_get_mode(void);
void state_set_mode(Mode_T);

#endif /* STATE_H */
