#include "config.h"

#include <asm-generic/errno-base.h>
#include <fuse/fuse.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "diofs.h"
#include "util.h"
#include "dentry.h"

void diofs_destroy(void *private_data) {
	diofs_d_freeall_child(diofs_root);
	diofs_i_freeall_next(diofs_inodes.start);
}

// Starts by creating a root directory and a file called "test_file" in it
void *diofs_init(struct fuse_conn_info *conn) {
	diofs_root = NEW(struct diofs_dentry);
	*diofs_root = (struct diofs_dentry) {
		.name = NULL,
		.ino = 1,
		.next_sib = NULL,
		.first_child = NULL
	};

	diofs_root->first_child = NEW(struct diofs_dentry);
	*(diofs_root->first_child) = (struct diofs_dentry) {
		.parent = diofs_root,
		.name = "test_file",
		.ino = 2,
		.first_child = NULL,
		.next_sib = NULL,
	};

	diofs_inodes.start = NEW(struct diofs_inode);
	*(diofs_inodes.start) = (struct diofs_inode) {
		.content = NULL,
		.size = 0,
		.nlink = 2,
		.ino = 1,
		.mode = S_IFDIR | 0755,
		.next = NULL
	};

	diofs_inodes.start->next = NEW(struct diofs_inode);
	*(diofs_inodes.start->next) = (struct diofs_inode) {
		.content = "this is a test file",
		.nlink = 1,
		.ino = 2,
		.mode = S_IFREG | 0777,
	};
	diofs_inodes.start->next->size
		= strlen(diofs_inodes.start->next->content);
}

int diofs_getattr(const char *path, struct stat *s) {
	struct diofs_inode *i = inode_from_path(path);
	if (i != NULL) {
		*s = (struct stat) {
			.st_ino = i->ino,
			.st_mode = i->mode,
			.st_nlink = i->nlink,
			.st_size = i->size
		};

		return 0;
	} else {
		return -ENOENT;
	}
}

int diofs_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info* fi) {
	(void) offset;
	(void) fi;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	struct diofs_dentry *cdentry = dentry_from_path(path);
	if (cdentry == NULL) return -ENOENT;

	struct diofs_inode *cinode = lookup_inode(cdentry->ino);
	if (!(cinode->mode & S_IFDIR)) return -ENOTDIR;

	for (cdentry = cdentry->first_child; cdentry != NULL;
			cdentry = cdentry->next_sib) {
		filler(buf, cdentry->name, NULL, 0);
	}

	return 0;
}

int diofs_open(const char *path, struct fuse_file_info *fi) {
	struct diofs_dentry *d = dentry_from_path(path);
	if (d == NULL)
		return -ENOENT;
	return 0;
}

int diofs_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	struct diofs_inode *i = inode_from_path(path);
	if (i != NULL) {
		if ((offset + size) <= i->size) {
			memcpy(buf, i->content + offset, size);
			return size;
		} else if (offset < i->size) {
			memcpy(buf, i->content + offset, i->size - offset);
			return i->size - offset;
		} else {
			return 0;
		}
	} else {
		return -ENOENT;
	}
}
