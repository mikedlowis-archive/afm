/**
  @file workdir.h
  @brief TODO: Describe this file
  $Revision$
  $HeadURL$
  */
#ifndef WORKDIR_H
#define WORKDIR_H

#include <stdbool.h>
#include "vec.h"

void workdir_deinit(void);

typedef struct {
    int idx;
    char* path;
    vec_t* vfiles;
} WorkDir_T;

typedef struct {
	char* path;
	char* name;
} File_T;

WorkDir_T* workdir_new(char* path);

void workdir_prev(WorkDir_T*);

void workdir_next(WorkDir_T*);

void workdir_cd(WorkDir_T*);

void workdir_ls(WorkDir_T*);

bool is_dir(char* path);

void workdir_seek(WorkDir_T* wd, char* search);

void workdir_scroll_to_top(WorkDir_T* wd);
void workdir_scroll_to_bot(WorkDir_T* wd);
void workdir_jump_down(WorkDir_T* wd);
void workdir_jump_up(WorkDir_T* wd);

#endif /* WORKDIR_H */
