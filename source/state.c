/* TODO: fix list.h */
#include <stddef.h> // needed because of a bug in list.h: size_t undefined.
#include <unistd.h>
#include "list.h"

#include "frame.h"
#include "state.h"
#include "screen.h"

/** Whether the system is currently running or not. */
static bool Running = true;

/** Whether the aardvark should be displayed */
static bool AardvarkOn = false;

/** A pointer to the currently focused node */
static list_node_t* Focused_Node  = NULL;

/** Current redraw state */
static RedrawState_T RedrawState = REDRAW_ALL_FRAMES;

/** Current operational mode */
static Mode_T CurrentMode = MODE_NORMAL;

bool state_get_running(void) {
    return Running;
}

void state_set_running(bool val) {
    Running = val;
}

bool state_get_aardvark_mode(void) {
    return AardvarkOn;
}

void state_set_aardvark_mode(bool val) {
    alarm(val ? 1 : 0);
    AardvarkOn = val;
}

list_node_t* state_get_focused_node(void) {
    return Focused_Node;
}

Frame_T* state_get_focused_frame(void) {
    return (NULL != Focused_Node) ? (Frame_T*)Focused_Node->contents : NULL;
}

WorkDir_T* state_get_focused_workdir(void) {
    Frame_T* focused_frame = state_get_focused_frame();
    return (NULL != focused_frame) ? focused_frame->workdir : NULL;
}

void state_set_focused_node(list_node_t *p_node) {
    frame_set_highlighting(state_get_focused_frame(), false, true);
    Focused_Node = p_node;
}

RedrawState_T state_get_redraw_state(void)
{
    return RedrawState;
}

void state_set_redraw_state(RedrawState_T state)
{
    if ((state == REDRAW_COMPLETE) || (RedrawState < state)) {
        RedrawState = state;
    }
}

Mode_T state_get_mode(void) {
    return CurrentMode;
}

void state_set_mode(Mode_T mode) {
    CurrentMode = mode;
}

