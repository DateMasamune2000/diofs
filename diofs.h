/*
 *    *** diofs.h ***
 *  Contains declarations for diofs methods. Exports a struct called diofs_ops
 *  that can be directly supplied to the fuse_main method.
 */
#ifndef _DIOFS_H
#define _DIOFS_H

#include "config.h"
#include <fuse/fuse.h>

extern struct fuse_operations diofs_ops;

void *diofs_init(struct fuse_conn_info *conn);
void diofs_destroy(void *private_data);
int diofs_getattr(const char *path, struct stat *s);
int diofs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi);
int diofs_open(const char *path, struct fuse_file_info *fi);
int diofs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

#endif // !_DIOFS_H
