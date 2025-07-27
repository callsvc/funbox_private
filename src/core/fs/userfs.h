#pragma once
#define FUSE_USE_VERSION 30
#include <fuse3/fuse_lowlevel.h>
#include <fs/types.h>
#include <algo/list.h>


typedef enum userfs_type {
    userfs_type_dir,
    userfs_type_file
} userfs_type_e;
typedef struct userfs_base {
    char mpath[100];
    fuse_ino_t inode_value;
    fuse_ino_t parent_inode;
    userfs_type_e type;
} userfs_base_t;

typedef struct userfs_file {
    userfs_base_t ubase;
    fsfile_t *file;
} userfs_file_t;

typedef struct userfs_dir {
    userfs_base_t ubase;
    list_t *children;
} userfs_dir_t;

typedef struct usefs {
    char mountname[100];
    struct fuse_session *se;

    userfs_dir_t *root_files;
    fuse_ino_t next_inode;
    uint8_t * buffer;

    pthread_t thread;
    volatile bool online;
} userfs_t;
userfs_t *userfs_create(const char *);
void userfs_mountfile(userfs_t*, fsfile_t*, const char*);
void userfs_destroy(userfs_t*);