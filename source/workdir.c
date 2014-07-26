/* external libraries */
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

/* internal libraries */
#include "vec.h"
#include "mem.h"

/* internal headers */
#include "state.h"
#include "workdir.h"
#include "screen.h"

static void get_files(int windex);

static bool is_dir(char* path) {
    struct stat s;
    return ((stat(path, &s) == 0) && (s.st_mode & S_IFDIR));
}

void workdir_free(void* p_wd);

WorkDir_T* workdir_new(char* path){
    WorkDir_T* wd = mem_allocate(sizeof(WorkDir_T), &workdir_free);
    wd->idx = 0;
    wd->path = path;
    mem_retain(path);
    wd->vfiles = vec_new(0);
    workdir_ls(wd);
    wd->top_index = 0;
    return wd;
}

void workdir_free(void* p_wd){
    WorkDir_T* wd = (WorkDir_T*)p_wd;
    fprintf(stderr, "freeing workdir\n");
    mem_release(wd->vfiles);
    mem_release(wd->path);
}

void workdir_next(WorkDir_T* wd) {
    //do nothing if at the end of the file list
    if(wd->idx < vec_size(wd->vfiles)-1){
        int rows,cols;
        wd->idx += 1;
        getmaxyx(stdscr, rows,cols);
        (void) cols;
        //scroll if necessary
        if((FrameTopBuffer+wd->idx+FrameBotBuffer) > rows)
            wd->top_index = wd->idx-(rows-FrameTopBuffer-FrameBotBuffer);
    }
    state_set_screen_dirty(true);
}

void workdir_prev(WorkDir_T* wd) {
    //do nothing if at the top of the file list
    if(wd->idx > 0){
        wd->idx -= 1;
        //scroll if necessary
        if(wd->idx < wd->top_index)
            wd->top_index = wd->idx;
    }
    state_set_screen_dirty(true);
}

//go up a directory: remove everything after (including) last '/' character
char* workdir_cd_up(WorkDir_T* wd){
    int last_slash = 0, i = 0;
    char* newpath;
    while(wd->path[i] != 0){
        if(wd->path[i] == '/') last_slash = i;
        i++;
    }
    if(last_slash == 0){
        newpath = mem_allocate(sizeof(char)*2, NULL);
        strcpy(newpath, "/");
    } else {
        newpath = mem_allocate(sizeof(char)*last_slash, NULL);
        strncpy(newpath, wd->path, last_slash);
        newpath[last_slash] = 0;
    }
    return newpath;
}

//go down a directory: append '/subdir' to path
char* workdir_cd_down(WorkDir_T* wd){
    char* subdir = vec_at(wd->vfiles, wd->idx);
    int newpathlen = strlen(wd->path) + strlen(subdir) + 2; //+2, for slash & end null;
    char *newpath = mem_allocate(sizeof(char)*newpathlen, NULL);
    strcpy(newpath, wd->path);
    strcat(newpath, "/");
    strcat(newpath, subdir);
    return newpath;
}

void workdir_cd(WorkDir_T* wd) {
    char* newpath = (wd->idx == 0) ? workdir_cd_up(wd) : workdir_cd_down(wd);
    if(is_dir(newpath)){
        mem_release(wd->path);
        wd->path = newpath;
        wd->idx = 0;
        wd->top_index = 0;
    }
    workdir_ls(wd);
    state_set_screen_dirty(true);
}

void workdir_ls(WorkDir_T* wd){
    char* dotdot = mem_allocate(sizeof(char) * 3, NULL);
    char* cmd = mem_allocate(sizeof(char) * (4+(strlen(wd->path))), NULL);
    size_t len = 0; //unused. reflects sized allocated for buffer (filename) by getline
    ssize_t read;
    char* filename = 0;
    FILE* ls;
    //free old file vector
    if(wd->vfiles) mem_release(wd->vfiles);
    //open new ls pipe
    strcpy(cmd, "ls ");
    strcat(cmd, wd->path);
    ls = popen(cmd, "r");
    strcpy(dotdot, "..");
    //initialize new file vector
    wd->vfiles = vec_new(1, dotdot); /* TODO: check if path = / */
    while ((read = getline(&filename, &len, ls)) != -1){
        char* lol = mem_allocate(read*sizeof(char), NULL);
        filename[read-1]=0; //remove ending newline
        strcpy(lol, filename);
        vec_push_back(wd->vfiles, lol);
        free(filename);
        filename = 0;
    }
    pclose(ls);
    //mem_release(dotdot); #dont free, because there's a bug(?) in vectors and reference counting
    //reference counter is not incremented for added items, so releasinghere will free the memory
    mem_release(cmd);
}

