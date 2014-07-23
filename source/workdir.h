/**
  @file workdir.h
  @brief TODO: Describe this file
  $Revision$
  $HeadURL$
  */
#ifndef WORKDIR_H
#define WORKDIR_H

#include "vec.h"

typedef struct {
    int idx;
    char cwd[1024];
    vec_t* vfiles;
    int top_index;
    char* title;
} WorkDir_T;

WorkDir_T* workdir_new(char* path);

void workdir_prev(WorkDir_T*);

void workdir_next(WorkDir_T*);

void workdir_cd(WorkDir_T*);

void workdir_refresh_file_list(WorkDir_T*);

#endif /* WORKDIR_H */
