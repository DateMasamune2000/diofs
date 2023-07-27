/*
 *    *** dentry.h ***
 *    Contains declarations of datatypes and functions for working with
 *    directory entires and inodes.
 */
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
	struct diofs_inode *next;
};

struct diofs_inode_list {
	struct diofs_inode *start;
};

extern struct diofs_dentry *diofs_root;
extern struct diofs_inode_list diofs_inodes;

void diofs_d_freeall_child(struct diofs_dentry *d);
void diofs_i_freeall_next(struct diofs_inode *n);
struct diofs_inode *lookup_inode(ino_t ino);
struct diofs_dentry *dentry_from_path(const char *path);
struct diofs_inode *inode_from_path(const char *path);
#endif
