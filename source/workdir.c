#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "vec.h"
#include "mem.h"
#include "state.h"
#include "workdir.h"
#include "screen.h"

static void get_files(int windex);

static bool is_dir(char* path) {
    struct stat s;
    if( stat(path, &s) == 0){
        return (s.st_mode & S_IFDIR);
    }/*else error*/
    return false;
    //TODO: oneliner: return ((stat(path, &s) == 0) && (s.st_mode & S_IFDIR));
}

void workdir_free(void* p_wd);

WorkDir_T* workdir_new(char* path){
	WorkDir_T* wd = mem_allocate(sizeof(WorkDir_T), &workdir_free);
	wd->idx = 0;
	wd->path = path;
	workdir_ls(wd);
	wd->top_index = 0;
	return wd;
}

void workdir_free(void* p_wd){
	WorkDir_T* wd = (WorkDir_T*)p_wd;
	mem_release(wd->vfiles);
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
		newpath[last_slash-1]=0;
	}
	mem_release(wd->path);
	wd->path = newpath;
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
	char* newpath  = (wd->idx == 0) ? workdir_cd_up(wd) : workdir_cd_down(wd);
	if(is_dir(newpath)){
		mem_release(wd->path);
		wd->path = newpath;
		wd->idx = 0;
		wd->top_index = 0;
	}
    //TODO: refresh file list
}

void workdir_ls(WorkDir_T* wd){
    int i=0;
    char* dotdot = mem_allocate(sizeof(char)*3, NULL);
    char cmd[1028] = "ls "; //TODO: suck less
    size_t len = 0; //unused. reflects sized allocated for buffer (filename) by getline
    ssize_t read;
    char* filename=0;
    FILE* ls;
    if(wd->vfiles) mem_release(wd->vfiles);
    strcpy(dotdot, "..");
    wd->vfiles = vec_new(1, dotdot); /* TODO: check if path = / */
    strcpy(&cmd[3], wd->path);
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

