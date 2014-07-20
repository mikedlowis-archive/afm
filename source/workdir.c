#include "workdir.h"
#include "state.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    int idx;
    char cwd[1024];
    char **files;
    int file_count;
    int top_index;
    char* title;
} Window_T;

/*TODO: arbitrary number of windows */
static Window_T Windows[1];

//number of lines to leave before/after dir contents
static int TopBuffer = 2;
static int BotBuffer = 2;

static void get_files(int windex);

static bool is_dir(char* path) {
    struct stat s;
    if( stat(path, &s) == 0){
        return (s.st_mode & S_IFDIR);
    }/*else error*/
    return false;
}

void workdir_init(int windex) {
    Windows[windex].idx = 0;
    getcwd(Windows[windex].cwd, 1024);
}

void workdir_next(void) {
    int index = state_get_focused_frame();
    //do nothing if at the end of the file list
    if(Windows[index].idx < Windows[index].file_count){
        Windows[index].idx += 1;
        int rows,cols;
        getmaxyx(stdscr, rows,cols);
        (void) cols;
        if((TopBuffer+Windows[index].idx+BotBuffer) > rows)
            Windows[index].top_index = Windows[index].idx-(rows-TopBuffer-BotBuffer);
    }
}

void workdir_prev(void) {
    int index = state_get_focused_frame();
    //do nothing if at the top of the file list
    if(Windows[index].idx > 0){
        Windows[index].idx -= 1;
        if(Windows[index].idx < Windows[index].top_index)
            Windows[index].top_index = Windows[index].idx;
    }
}

void workdir_cd(void) {
    int windex = state_get_focused_frame();
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

void workdir_ls(void) {
    int windex = state_get_focused_frame();
    get_files(windex);
    int i = Windows[windex].top_index;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    attron(A_UNDERLINE);
    mvaddnstr(1, 1, Windows[windex].cwd, cols-2);
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

static void get_files(int windex){
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
