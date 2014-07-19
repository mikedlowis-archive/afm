#include <ncurses.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> /*TODO: anything using this is almost certainly broken on windows */
#include <string.h>
#include <sys/stat.h>

typedef struct {
    int idx;
    char cwd[1024];
    char **files;
    int file_count;
    int top_index;
    char* title;
} Window_T;

typedef struct {
	char a, b, c;
} Triplet_T;

static bool Running = true;
static bool Screen_Dirty = true;
static bool Resized = true;
/*TODO: arbitrary number of windows */
static Window_T Windows[1];
static int FocusedWindex = 0;
static bool AardvarkOn = false;

//number of lines to leave before/after dir contents
static int TopBuffer = 2;
static int BotBuffer = 2;

static Triplet_T aadata[77] = {
	{  0, 34, 14 }, {  1, 30, 22 }, {  2, 17,  2 }, {  2, 28, 26 },
	{  3,  3,  2 }, {  3, 16,  4 }, {  3, 26, 30 }, {  4,  3,  3 }, {  4, 15,  5 }, {  4, 24, 33 },
	{  5,  4,  3 }, {  5, 15,  5 }, {  5, 22, 37 }, {  6,  4,  5 }, {  6, 15, 45 },
	{  7,  5,  5 }, {  7, 14, 47 }, {  8,  6,  5 }, {  8, 14, 48 }, {  9,  7,  6 }, {  9, 14, 48 },
	{ 10,  9, 54 }, { 11,  9, 54 }, { 12,  8, 56 }, { 13,  7,  5 }, { 13, 14, 50 },
	{ 14,  6, 58 }, { 15,  6, 59 }, { 16,  5, 60 }, { 17,  4, 12 }, { 17, 19, 39 }, { 17, 59,  6 },
	{ 18,  3, 10 }, { 18, 20, 17 }, { 18, 39,  9 }, { 18, 49,  7 }, { 18, 60,  5 },
	{ 19,  1, 10 }, { 19, 20, 17 }, { 19, 40,  8 }, { 19, 50,  6 }, { 19, 61,  5 },
	{ 20,  0,  8 }, { 20, 21, 15 }, { 20, 42,  6 }, { 20, 50,  6 }, { 20, 62,  4 },
	{ 21,  0,  7 }, { 21, 22,  5 }, { 21, 29,  7 }, { 21, 42,  5 }, { 21, 50,  5 }, { 21, 63,  4 },
	{ 22,  2,  3 }, { 22, 22,  5 }, { 22, 30,  6 }, { 22, 41,  5 }, { 22, 50,  6 }, { 22, 64,  3 },
	{ 23, 23,  5 }, { 23, 30,  7 }, { 23, 40,  5 }, { 23, 50,  6 }, { 23, 65,  4 },
	{ 24, 23,  5 }, { 24, 31,  6 }, { 24, 38,  7 }, { 24, 50,  6 }, { 24, 66,  4 },
	{ 25, 21,  6 }, { 25, 30, 14 }, { 25, 49,  7 }, { 25, 68,  2 },
	{ 26, 18,  8 }, { 26, 27, 15 }, { 26, 49,  7 }, { 27, 26,  6 }
};

void draw_aardvark(){
	int row, col;
	getmaxyx(stdscr, row, col);
	int i;
	int trips=sizeof(aadata)/sizeof(Triplet_T);
	char* aardvark="############################################################";
	for(i=0; i<trips; i++)
		mvaddnstr(((row-28)/2)+aadata[i].a, (col-70)/2+aadata[i].b, aardvark, aadata[i].c);
	refresh();
}
bool is_dir(char* path){
    struct stat s;
    if( stat(path, &s) == 0){
        return (s.st_mode & S_IFDIR);
    }/*else error*/
    return false;
}

/* TODO redraw less frequently */
/* TODO detect filesystem changes */

void get_files(int windex){
    /*free existing contents*/
    int i=0;
    if(Windows[windex].files){
        /*fuck memory (this is broken)
        while(Files[i]){
            free(Files[i]);
            i++;
        }*/
        free(Windows[windex].files);
    }
    /* TODO: malloc smartly, instead of tapping out at 1024 files */
    Windows[windex].files = malloc(sizeof(char*) * 1024);
    Windows[windex].files[0] = ".."; /* parent directory; TODO only add if cwd!=/ */
    char cmd[1028] = "ls ";
    strcpy(&cmd[3], Windows[windex].cwd);
    FILE* ls = popen(cmd, "r");
    size_t len = 0;
    ssize_t read;
    i = 1;
    while ((read = getline(&Windows[windex].files[i], &len, ls)) != -1){
        if(Windows[windex].files[i][read-1] == '\n') Windows[windex].files[i][read-1] = 0;
        i++;
        if(i>1022) break;
    }
    Windows[windex].file_count = i-1;
    Windows[windex].files[i] = 0; /*always end with nullpointer; since file_count is a thing, can probably do without this*/
}

void cd(int windex){
    int last_slash=0, i=0;
    bool ends_with_slash = false;
    while(Windows[windex].cwd[i] != 0){
        if(Windows[windex].cwd[i] == '/')
            last_slash = i;
        i++;
    }
    ends_with_slash = (last_slash == (i-1)); /* should only be true for root */
    if(Windows[windex].idx == 0) { /* up */
        //truncate cwd including the last slash
        Windows[windex].cwd[last_slash]=0;
        if(last_slash==0){ //at root. fixitfixitfixit.
            Windows[windex].cwd[0]='/';
            Windows[windex].cwd[1]=0;
        }
    }else{
        //add file to cwd:
        int cwdend = i;
        if(!ends_with_slash){
            Windows[windex].cwd[i] = '/';
            i++;
        }
        strcpy(&Windows[windex].cwd[i], Windows[windex].files[Windows[windex].idx]);
        Windows[windex].idx = 0;
        Windows[windex].top_index = 0;
        //if not a directory, revert
        if(!is_dir(Windows[windex].cwd)) Windows[windex].cwd[cwdend]=0;
    }
}

void list_files(int windex) {
    get_files(windex);
    int i = Windows[windex].top_index;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    attron(A_UNDERLINE);
    mvaddnstr(1, 1, &Windows[windex].cwd, cols-2);
    attroff(A_UNDERLINE);
    while (Windows[windex].files[i] != 0){
        if(i==Windows[windex].idx){
            attron(A_STANDOUT);
            attron(A_BOLD);
        }
        mvaddnstr(TopBuffer+i-Windows[windex].top_index, 1, Windows[windex].files[i], cols-2);
        if(i == Windows[windex].idx){
            attroff(A_STANDOUT);
            attroff(A_BOLD);
        }
        i++;
        if((TopBuffer+i-Windows[windex].top_index+BotBuffer) > rows) break;
    }
}

void scroll_down(){
    //do nothing if at the end of the file list
    if(Windows[FocusedWindex].idx < Windows[FocusedWindex].file_count){
        Windows[FocusedWindex].idx += 1;
        int rows,cols;
        getmaxyx(stdscr, rows,cols);
        (void) cols;
        if((TopBuffer+Windows[FocusedWindex].idx+BotBuffer) > rows)
            Windows[FocusedWindex].top_index = Windows[FocusedWindex].idx-(rows-TopBuffer-BotBuffer);
    }
}
void scroll_up(){
    //do nothing if at the top of the file list
    if(Windows[FocusedWindex].idx > 0){
        Windows[FocusedWindex].idx -= 1;
        if(Windows[FocusedWindex].idx < Windows[FocusedWindex].top_index)
            Windows[FocusedWindex].top_index = Windows[FocusedWindex].idx;
    }
}

void update_screen(void) {
    /* Clear screen and update LINES and COLS */
    if(Resized){
        endwin();
        Resized = false;
    }
    clear();
    //should probably redraw all, but since only one window exists, it doesn't matter
    list_files(FocusedWindex);
    if(AardvarkOn) draw_aardvark();
    /* Draw the Border */
    mvaddch(0,       0,      ACS_ULCORNER);
    mvhline(0,       1,      ACS_HLINE, COLS-2);
    mvaddch(0,       COLS-1, ACS_URCORNER);
    mvvline(1,       0,      ACS_VLINE, LINES-2);
    mvaddch(LINES-1, 0,      ACS_LLCORNER);
    mvhline(LINES-1, 1,      ACS_HLINE, COLS-2);
    mvaddch(LINES-1, COLS-1, ACS_LRCORNER);
    mvvline(1,       COLS-1, ACS_VLINE, LINES-2);
    /* Refresh and mark complete */
    refresh();
    Screen_Dirty = false;
}

void handle_input(char ch) {
    /* Assume screen is dirty by default */
    bool is_screen_dirty = true;
    switch(ch){
        case 'a': AardvarkOn = !AardvarkOn;
                  break;
        case 'q': Running = false;
                  break;
        case 'j': scroll_down();
                  break;
        case 'k': scroll_up();
                  break;
        case 'e': cd(FocusedWindex);
                  break;
        default:  is_screen_dirty = false;
                  break;
    }
    Screen_Dirty = Screen_Dirty || is_screen_dirty;
}

void handle_signal(int sig) {
    signal(SIGWINCH, handle_signal);
    Screen_Dirty = true;
    Resized = true;
}

void init_window_t(windex){
    Windows[windex].idx = 0;
    getcwd(Windows[windex].cwd, 1024);
}
    
int main(int argc, char** argv) {
    init_window_t(0);
    /* Handle terminal resizing */
    signal(SIGWINCH, handle_signal);
    /* Initialize ncurses and user input settings */
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    timeout(25);
    refresh();
    while(Running) {
        if(Screen_Dirty) update_screen();
        handle_input(getch());
    }
    erase();
    refresh();
    endwin();
    return 0;
}

