#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "vec.h"
#include "mem.h"
#include "state.h"
#include "workdir.h"

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

void workdir_free(void* p_wd);

WorkDir_T* workdir_new(char* path){
	WorkDir_T* wd = mem_allocate(sizeof(WorkDir_T), workdir_free);
	wd->idx = 0;
	strcpy(wd->cwd, path);
	wd->vfiles = vec_new(0);
	wd->top_index = 0;
	return wd;
}

void workdir_free(void* p_wd){
	//TODO: free shit.
}

/*
void workdir_init(int windex) {
    Windows[windex].idx = 0;
    getcwd(Windows[windex].cwd, 1024);
    Windows[windex].vfiles = vec_new(0);
}
*/


void workdir_next(WorkDir_T* wd) {
    //do nothing if at the end of the file list
    if(wd->idx < vec_size(wd->vfiles)-1){
        int rows,cols;
        wd->idx += 1;
        getmaxyx(stdscr, rows,cols);
        (void) cols;
        //scroll if necessary
        if((TopBuffer+wd->idx+BotBuffer) > rows)
            wd->top_index = wd->idx-(rows-TopBuffer-BotBuffer);
    }
}

void workdir_prev(WorkDir_T* wd) {
    //do nothing if at the top of the file list
    if(wd->idx > 0){
        wd->idx -= 1;
        //scroll if necessary
        if(wd->idx < wd->top_index)
            wd->top_index = wd->idx;
    }
}

void workdir_cd(WorkDir_T* wd) {
    int last_slash=0, i=0;
    bool ends_with_slash = false;
    while(wd->cwd[i] != 0){
        if(wd->cwd[i] == '/')
            last_slash = i;
        i++;
    }
    ends_with_slash = (last_slash == (i-1)); /* should only be true for root */
    if(wd->idx == 0) { /* up */
        //truncate cwd including the last slash
        wd->cwd[last_slash]=0;
        if(last_slash==0){ //at root. fixitfixitfixit.
			strcpy(wd->cwd, "/");
            //wd->cwd[0]='/';
            //wd->cwd[1]=0;
        }
    }else{
        //add file to cwd:
        int cwdend = i;
        if(!ends_with_slash){
            wd->cwd[i] = '/';
            i++;
        }
        strcpy(&(wd->cwd[i]), vec_at(wd->vfiles, wd->idx));
        wd->idx = 0;
        wd->top_index = 0;
        //if not a directory, revert
        if(!is_dir(wd->cwd)) wd->cwd[cwdend]=0;
    }
    //TODO: refresh file list
}

/*
void workdir_ls(void) {
    int windex = state_get_focused_frame();
    int i = Windows[windex].top_index;
    int rows, cols;
    get_files(windex);
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
*/

void workdir_refresh_file_list(WorkDir_T* wd){
    int i=0;
    char* dotdot = mem_allocate(sizeof(char)*3, NULL);
    char cmd[1028] = "ls "; //TODO: suck less
    size_t len = 0; //unused. reflects sized allocated for buffer (filename) by getline
    ssize_t read;
    char* filename=0;
    FILE* ls;
    if(wd->vfiles) mem_release(wd->vfiles);
    strcpy(dotdot, "..");
    wd->vfiles = vec_new(1, dotdot); /* TODO: check if cwd = / */
    strcpy(&cmd[3], wd->cwd);
    ls = popen(cmd, "r");
    i = 1;
    while ((read = getline(&filename, &len, ls)) != -1){
        char* lol = mem_allocate(read*sizeof(char), NULL);
        filename[read-1]=0; //remove ending newline
        strcpy(lol, filename);
        vec_push_back(wd->vfiles, lol);
        i++;
        if(i>1022) break;
    }
}

