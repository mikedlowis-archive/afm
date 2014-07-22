#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <vec.h>
#include <mem.h>

#include "state.h"
#include "workdir.h"

typedef struct {
    int idx;
    char cwd[1024];
    vec_t* vfiles;
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
    Windows[windex].vfiles = vec_new(0);
}

void workdir_next(void) {
    int index = state_get_focused_frame();
    //do nothing if at the end of the file list
    if(Windows[index].idx < vec_size(Windows[index].vfiles)-1){
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
        strcpy(&Windows[windex].cwd[i], vec_at(Windows[windex].vfiles, Windows[windex].idx));
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
    while (i < vec_size(Windows[windex].vfiles)){
        if(i == Windows[windex].idx){
            attron(A_STANDOUT);
            attron(A_BOLD);
        }
        mvaddnstr(TopBuffer+i-Windows[windex].top_index, 1, vec_at(Windows[windex].vfiles, i), cols-2);
        if(i == Windows[windex].idx){
            attroff(A_STANDOUT);
            attroff(A_BOLD);
        }
        i++;
        if((TopBuffer+i-Windows[windex].top_index+BotBuffer) > rows) break;
    }
}

static void get_files(int windex){
    int i=0;
    if(Windows[windex].vfiles) mem_release(Windows[windex].vfiles);
    char* dotdot = mem_allocate(sizeof(char)*3, NULL);
    strcpy(dotdot, "..");
    Windows[windex].vfiles = vec_new(1, dotdot); /* TODO: check if cwd = / */
    char cmd[1028] = "ls ";
    strcpy(&cmd[3], Windows[windex].cwd);
    FILE* ls = popen(cmd, "r");
    size_t len = 0; //unused. reflects sized allocated for buffer (filename) by getline
    ssize_t read;
    char* filename=0;
    i = 1;
    while ((read = getline(&filename, &len, ls)) != -1){
        filename[read-1]=0; //remove ending newline
        char* lol = mem_allocate(read*sizeof(char), NULL);
        strcpy(lol, filename);
        vec_push_back(Windows[windex].vfiles, lol);
        i++;
        if(i>1022) break;
    }
}

