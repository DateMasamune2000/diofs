#ifndef _DIOFS_H
#define _DIOFS_H

#include "config.h"
#include <fuse/fuse.h>

struct diofs_dentry {
	char *name;

	unsigned int ino;

	struct diofs_dentry *parent;
	struct diofs_dentry *first_child;
	struct diofs_dentry *next_sib;
};

struct diofs_inode {
	void *content;
	off_t size;
	ino_t ino;
	nlink_t nlink;
	mode_t mode;
};

void *diofs_init(struct fuse_conn_info *conn);
int diofs_getattr(const char *path, struct stat *s);
int diofs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi);
int diofs_open(const char *path, struct fuse_file_info *fi);
int diofs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

#endif // !_DIOFS_H
