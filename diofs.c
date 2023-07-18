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

static struct diofs_dentry *diofs_root;
static struct diofs_inode_list diofs_inodes;

void diofs_d_freeall_child(struct diofs_dentry *d) {
	if (d->first_child != NULL)
		diofs_d_freeall_child(d->first_child);

	if (d->next_sib != NULL)
		diofs_d_freeall_child(d->next_sib);

	free(d);
}

void diofs_i_freeall_next(struct diofs_inode *n) {
	if (n->next != NULL)
		diofs_i_freeall_next(n->next);
	free(n);
}

void diofs_destroy(void *private_data) {
	diofs_d_freeall_child(diofs_root);
	diofs_i_freeall_next(diofs_inodes.start);
}

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

struct diofs_inode *lookup_inode(ino_t ino) {
	for (struct diofs_inode *i = diofs_inodes.start;
			i != NULL;
			i = i->next) {
		if (i->ino == ino) return i;
	}

	return NULL;
}

struct diofs_dentry *dentry_from_path(const char *path) {
	// If the root is asked for, just return it. There is probably a more
	// streamlined way to do this, but this works for now.
	if (new_strcmp(path, "/") == STR_EQUAL) {
		return diofs_root;
	}

	// NOTE: This part is very inefficient. It may need to be sped up later.
	char *path_dup = strdup(path);
	char *token, *saveptr1;

	// NOTE: This part runs fine for now, but a cache might be needed later.
	struct diofs_dentry *cfile = diofs_root->first_child, *cfile_prev = NULL;

	// Split the path on '/', and check if the path is valid by walking the
	// tree step-by-step
	token = strtok_r(path_dup, "/", &saveptr1);
	while (token != NULL) {
		if (cfile == NULL) goto cleanup;

		for (; cfile != NULL; cfile = cfile->next_sib) {
			if (new_strcmp(cfile->name, token) == STR_EQUAL) {
				cfile_prev = cfile;
				cfile = cfile->first_child;
				token = strtok_r(NULL, "/", &saveptr1);
				break;
			}
		}
	}

cleanup:
	// TODO: There's probably a better way to do this.
	while (token != NULL) { token = strtok_r(NULL, "/", &saveptr1); }

	free(path_dup);

	return cfile_prev;
}

struct diofs_inode *inode_from_path(const char *path) {
	struct diofs_dentry *dentry = dentry_from_path(path);
	if (dentry == NULL) {
		return NULL;
	} else {
		return lookup_inode(dentry->ino);
	}
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
