#ifndef _DIOFS_DENTRY_H
#define _DIOFS_DENTRY_H

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

#endif
