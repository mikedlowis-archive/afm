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

bool is_dir(char* path) {
    struct stat s;
    return ((stat(path, &s) == 0) && (s.st_mode & S_IFDIR));
}

void workdir_free(void* p_wd);

WorkDir_T* workdir_new(char* path){
    WorkDir_T* wd = mem_allocate(sizeof(WorkDir_T), &workdir_free);
    wd->idx = 0;
    wd->path = path;
    mem_retain(wd->path);
    wd->vfiles = vec_new(0);
    workdir_ls(wd);
    wd->top_index = 0;
    return wd;
}

void workdir_free(void* p_wd){
    WorkDir_T* wd = (WorkDir_T*)p_wd;
    mem_release(wd->vfiles);
    mem_release(wd->path);
}

void file_free(void* p_vfile){
    File_T* p_file = (File_T*) p_vfile;
    //only free name if == special value '..'. else will be location in path
    if(strcmp(p_file->name, "..") == 0) mem_release(p_file->name);
    mem_release(p_file->path);
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

void workdir_cd(WorkDir_T* wd) {
    char* newpath = ((File_T*) vec_at(wd->vfiles, wd->idx))->path;
    if(is_dir(newpath)){
        mem_release(wd->path);
        wd->path = newpath;
        mem_retain(wd->path);
        wd->idx = 0;
        wd->top_index = 0;
    }
    workdir_ls(wd);
    state_set_screen_dirty(true);
}

File_T* make_dotdot(char* path){
    File_T* dd = NULL;
    int last_slash = 0;
    if(strcmp(path, "/") != 0){
        dd = mem_allocate(sizeof(File_T), &file_free);
        dd->name = mem_allocate(sizeof(char)*3, NULL);
        strcpy(dd->name, "..");
        for(int i=0; path[i] != 0; i++){
            if(path[i] == '/') last_slash = i;
        }
        if(last_slash == 0){
            dd->path = mem_allocate(sizeof(char)*2, NULL);
            strcpy(dd->path, "/");
        } else {
            dd->path = mem_allocate(sizeof(char)*(1+last_slash), NULL);
            strncpy(dd->path, path, last_slash);
            dd->path[last_slash] = 0;
        }
    }
    return dd;
}

char* ls_command(char* path){
    char* cmd = mem_allocate(sizeof(char) * (4+(strlen(path))), NULL);
    strcpy(cmd, "ls ");
    strcat(cmd, path);
}

void workdir_ls(WorkDir_T* wd){
    File_T* dd = make_dotdot(wd->path);
    char* cmd = ls_command(wd->path);
    size_t len = 0; //unused. reflects sized allocated for buffer (filename) by getline
    ssize_t read;
    char* filename = 0;
    FILE* ls;
    int pathlength = strlen(wd->path);
    //free old file vector
    if(wd->vfiles) mem_release(wd->vfiles);
    //open new ls pipe
    ls = popen(cmd, "r");
    //initialize new file vector
    wd->vfiles = vec_new(0);
    if(dd) vec_push_back(wd->vfiles, dd);
    while ((read = getline(&filename, &len, ls)) != -1){
        File_T* file = mem_allocate(sizeof(File_T), &file_free);
        file->path = mem_allocate((pathlength+read+1)*sizeof(char), NULL);
        int filename_offset = pathlength;
        filename[read-1]=0; //remove ending newline
        //build full path:
        strcpy(file->path, wd->path);
        if (wd->path[pathlength-1] != '/') {
            strcat(file->path, "/");
            filename_offset += 1;
        }
        strcat(file->path, filename);
        file->name = &(file->path[filename_offset]);
        vec_push_back(wd->vfiles, file);
    }
    free(filename);
    pclose(ls);
    mem_release(cmd);
}

