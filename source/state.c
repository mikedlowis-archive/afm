/* TODO: fix list.h */
#include <stddef.h> // needed because of a bug in list.h: size_t undefined.
#include "list.h"

#include "frame.h"
#include "state.h"
#include "screen.h"

/** Whether the system is currently running or not. */
static bool Running = true;

/** Whether the screen should be refreshed or not. */
static bool Screen_Dirty = true;

/** Whether the terminal has been resized or not. */
static bool Resized = true;

/** Whether the aardvark should be displayed */
static bool AardvarkOn = false;

/** A pointer to the currently focused node */
static list_node_t* Focused_Node  = NULL;

static Mode_T CurrentMode = 0;

bool state_get_running(void) {
    return Running;
}

void state_set_running(bool val) {
    Running = val;
}

bool state_get_screen_dirty(void) {
    return Screen_Dirty;
}

void state_set_screen_dirty(bool val) {
    Screen_Dirty = val;
}

bool state_get_screen_resized(void) {
    return Resized;
}

void state_set_screen_resized(bool val) {
    Resized = val;
}

bool state_get_aardvark_mode(void) {
    return AardvarkOn;
}

void state_set_aardvark_mode(bool val) {
    AardvarkOn = val;
}

list_node_t* state_get_focused_node(void) {
    return Focused_Node;
}

Frame_T* state_get_focused_frame(void) {
    return Focused_Node ? (Frame_T*)Focused_Node->contents : NULL;
}

WorkDir_T* state_get_focused_workdir(void) {
    Frame_T* focused_frame = state_get_focused_frame();
    return focused_frame ? focused_frame->workdir : NULL;
}

void state_set_focused_node(list_node_t *p_node) {
    Focused_Node = p_node;
}

Mode_T state_get_mode() {
    return CurrentMode;
}

void state_set_mode(Mode_T m) {
    CurrentMode = m;
}

