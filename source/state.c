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

/** A pointer to the currently focused frame */
static Frame_T* Focused_Frame = 0;

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

Frame_T* state_get_focused_frame(void) {
    return Focused_Frame;
}

WorkDir_T* state_get_focused_workdir(void) {
	return Focused_Frame->workdir;
}

void state_set_focused_frame(Frame_T *p_frame) {
    Focused_Frame = p_frame;
}

Mode_T state_get_mode() {
    return CurrentMode;
}

void state_set_mode(Mode_T m) {
    CurrentMode = m;
}

