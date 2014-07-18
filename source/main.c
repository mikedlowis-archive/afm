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
static WINDOW* WindowLeft;
static WINDOW* WindowRight;

static int Idx = 0; /* TODO: this should be per-window */
static char Cwd[1024]; /* TODO; this should be per-window & smarter than 1024chars */
static char** Files; /* TODO: this should be per-window */
static int FileCount; /* TODO: ditto */

bool is_dir(char* path){
    struct stat s;
    if( stat(path, &s) == 0){
        return (s.st_mode & S_IFDIR);
    }/*else error*/
    return false;
}

/* TODO redraw less frequently */
/* TODO detect filesystem changes */

/*TODO: when have collection of windows and ^^ vars are in a struct, should take a windex*/
void get_files(){
    /*free existing contents*/
    int i=0;
    if(Files){
        /*fuck memory (this is broken)
        while(Files[i]){
            free(Files[i]);
            i++;
        }*/
        free(Files);
    }
    /* TODO: malloc smartly, instead of tapping out at 1024 files */
    Files = malloc(sizeof(char*)*1024);
    Files[0] = ".."; /* parent directory; todo only add if cwd!=/ */
    char cmd[1028]="ls ";
    strcpy(&cmd[3],Cwd);
    FILE* ls = popen(cmd, "r");
    size_t len = 0;
    ssize_t read;
    i=1;
    while ((read = getline(&Files[i], &len, ls)) != -1){
        if(Files[i][read-1] == '\n') Files[i][read-1] = 0;
        i++;
        if(i>1022) break;
    }
    FileCount=i-1;
    Files[i]=0; /*always end with nullpointer */
}

void cd(int file_index){
    int last_slash=0, i=0;
    bool ends_with_slash = false;
    while(Cwd[i] != 0){
        if(Cwd[i] == '/')
            last_slash=i;
        i++;
    }
    if(last_slash==i-1) { /* should only be true for root */
        ends_with_slash = true;
    }
    if(file_index==0) { /* up */
        //truncate Cwd including the last slash
        Cwd[last_slash]=0;
        if(last_slash==0){ /*but dont have an empty cwd */
            Cwd[0]='/';
            Cwd[1]=0;
        }
    }else{
        //add file to Cwd:
        int cwdend = i;
        if(!ends_with_slash){
            Cwd[i] = '/';
            i++;
        }
        (void)cwdend;
        strcpy(&Cwd[i], Files[file_index]);
        //kill_newlines(Cwd);
        Idx=0;
        //if not a directory, revert
        if(!is_dir(Cwd)) Cwd[cwdend]=0;
    }
}

void list_files(WINDOW* win) {
    get_files();
    int topbuff=2; /* lines to skip before printing (window border/title) */
    int i = 0;
    int rows, cols;
    getmaxyx(win, rows, cols);
    wattron(win, A_UNDERLINE);
    mvwaddnstr(win, 1, 1, &Cwd, cols-2);
    wattroff(win, A_UNDERLINE);
    while (Files[i] != 0){
        if(i==Idx){
            wattron(win, A_STANDOUT);
            wattron(win, A_BOLD);
        }
        mvwaddnstr(win, i+topbuff, 1, Files[i], cols-2); /* prevent spilling out of window with long filenames */
        if(i==Idx){
            wattroff(win, A_STANDOUT);
            wattroff(win, A_BOLD);
        }
        i++;
        if((topbuff+i)>rows) break; /* TODO: implement scrolling to handle when there are more files than lines */
    }
}

void handle_input(char ch) {
    if(ch == 'q')
        Running = false;
    if(ch == 'j' && Idx<FileCount)
        Idx+=1;
    if(ch == 'k' && Idx>0)
        Idx-=1;
    if(ch == 'e')
        cd(Idx);
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
    WindowLeft  = create_window(0,0,LINES,COLS/2);
    wprintw(WindowLeft,  "\rLeft");
    list_files(WindowLeft);
    wrefresh(WindowLeft);
    WindowRight = create_window(COLS/2,0,LINES,COLS/2);
    wprintw(WindowRight, "\rRight");
    wrefresh(WindowRight);
}

void handle_signal(int sig) {
    update_screen();
    signal(SIGWINCH, handle_signal);
}

int main(int argc, char** argv) {
    getcwd(Cwd, sizeof(Cwd)); /* TODO: check for errors; fix in windows */
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
    destroy_window(WindowLeft);
    destroy_window(WindowRight);
    endwin();
    return 0;
}
