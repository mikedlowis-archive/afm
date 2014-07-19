#include <ncurses.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> /*TODO: anything using this is almost certainly broken on windows */
#include <string.h>
#include <sys/stat.h>

static bool Running = true;
//static int Term_X = 0;
//static int Term_Y = 0;

typedef struct {
    int idx;
    char cwd[1024];
    char **files;
    int file_count;
    int top_index;
    WINDOW* win;
} window_t;

/*todo: arbitrary number of windows */
static window_t Windows[2];
static int FocusedWindex = 0;

//number of lines to leave before/after dir contents
static int TopBuffer = 2;
static int BotBuffer = 2;

bool is_dir(char* path){
    struct stat s;
    if( stat(path, &s) == 0){
        return (s.st_mode & S_IFDIR);
    }/*else error*/
    /*TODO: handle symlinks*/
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
    getmaxyx(Windows[windex].win, rows, cols);
    wattron(Windows[windex].win, A_UNDERLINE);
    mvwaddnstr(Windows[windex].win, 1, 1, &Windows[windex].cwd, cols-2);
    wattroff(Windows[windex].win, A_UNDERLINE);
    while (Windows[windex].files[i] != 0){
        if(i==Windows[windex].idx){
            wattron(Windows[windex].win, A_STANDOUT);
            wattron(Windows[windex].win, A_BOLD);
        }
        mvwaddnstr(Windows[windex].win, TopBuffer+i-Windows[windex].top_index, 1, Windows[windex].files[i], cols-2);
        if(i == Windows[windex].idx){
            wattroff(Windows[windex].win, A_STANDOUT);
            wattroff(Windows[windex].win, A_BOLD);
        }
        i++;
        if((TopBuffer+i-Windows[windex].top_index+BotBuffer) > rows) break;
    }
}

void handle_input(char ch) {
    if(ch == 'q')
        Running = false;
    if(ch == 'j' && Windows[FocusedWindex].idx < Windows[FocusedWindex].file_count){
        Windows[FocusedWindex].idx += 1;
        int rows,cols;
        getmaxyx(stdscr, rows,cols);
        (void) cols;
        if((TopBuffer+Windows[FocusedWindex].idx+BotBuffer) > rows)
            Windows[FocusedWindex].top_index = Windows[FocusedWindex].idx-(rows-TopBuffer-BotBuffer);
    }
    if(ch == 'k' && Windows[FocusedWindex].idx > 0){
        Windows[FocusedWindex].idx -= 1;
        if(Windows[FocusedWindex].idx < Windows[FocusedWindex].top_index)
            Windows[FocusedWindex].top_index = Windows[FocusedWindex].idx;
    }
    if(ch == 'e')
        cd(FocusedWindex);
}

WINDOW* create_window(int x, int y, int height, int width) {
    WINDOW* p_win = newwin(height, width, y, x);
    box(p_win, 0, 0);
    wrefresh(p_win);
    return p_win;
}

void destroy_window(WINDOW* p_win) {
    /* Erase remnants of the window */
    wborder(p_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(p_win);
    /* Delete the window */
    delwin(p_win);
}

void update_screen(void) {
    endwin();
    /* Create the left and right windows */
    Windows[0].win  = create_window(0,0,LINES,COLS/2);
    wprintw(Windows[0].win,  "\rLeft");
    list_files(0);
    wrefresh(Windows[0].win);
    Windows[1].win = create_window(COLS/2,0,LINES,COLS/2);
    wprintw(Windows[1].win, "\rRight");
    wrefresh(Windows[1].win);
}

void handle_signal(int sig) {
    update_screen();
    signal(SIGWINCH, handle_signal);
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
    refresh();
    while(Running) {
        update_screen();
        handle_input(getch());
    }
    destroy_window(Windows[0].win);
    destroy_window(Windows[1].win);
    endwin();
    return 0;
}
