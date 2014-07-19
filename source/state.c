#include "state.h"

/** Whether the system is currently running or not. */
static bool Running = true;

/** Whether the screen should be refreshed or not. */
static bool Screen_Dirty = true;

/** Whether the terminal has been resized or not. */
static bool Resized = true;

/** Whether the aardvark should be displayed */
static bool AardvarkOn = false;

/** The currently focused frame id */
static int Focused_Index = 0;

bool state_get_running(void)
{
    return Running;
}

void state_set_running(bool val)
{
    Running = val;
}

bool state_get_screen_dirty(void)
{
    return Screen_Dirty;
}

void state_set_screen_dirty(bool val)
{
    Screen_Dirty = val;
}

bool state_get_screen_resized(void)
{
    return Resized;
}

void state_set_screen_resized(bool val)
{
    Resized = val;
}

bool state_get_aardvark_mode(void)
{
    return AardvarkOn;
}

void state_set_aardvark_mode(bool val)
{
    AardvarkOn = val;
}

int state_get_focused_frame(void)
{
    return Focused_Index;
}

void state_set_focused_frame(int id)
{
    Focused_Index = id;
}

