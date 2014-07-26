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
    int top_index;
} WorkDir_T;

WorkDir_T* workdir_new(char* path);

void workdir_prev(WorkDir_T*);

void workdir_next(WorkDir_T*);

void workdir_cd(WorkDir_T*);

void workdir_ls(WorkDir_T*);

bool is_dir(char* path);

#endif /* WORKDIR_H */
