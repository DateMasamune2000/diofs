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

static struct diofs_dentry diofs_test_file;
static struct diofs_dentry diofs_root;

static struct diofs_inode diofs_root_inode;
static struct diofs_inode diofs_test_inode;

char temp_filename[] = "test_file";
char temp_path[] = "/test_file";
char temp_contents[] = "this is a test file\n\0";
int temp_len = sizeof(temp_contents)/sizeof(char);

void *diofs_init(struct fuse_conn_info *conn) {
	assert(&diofs_root != NULL);
	assert(&diofs_test_file != NULL);
	assert(&diofs_root_inode != NULL);
	assert(&diofs_test_inode != NULL);

	diofs_root.parent = &diofs_root;
	diofs_root.name = NULL;
	diofs_root.ino = 1;
	diofs_root.first_child = &diofs_test_file;
	diofs_root.next_sib = NULL;

	diofs_test_file.parent = &diofs_root;
	diofs_test_file.name = temp_filename;
	diofs_test_file.ino = 2;
	diofs_test_file.first_child = NULL;
	diofs_test_file.next_sib = NULL;

	diofs_root_inode.content = NULL;
	diofs_root_inode.size = 0;
	diofs_root_inode.nlink = 2;
	diofs_root_inode.ino = 1;
	diofs_root_inode.mode = S_IFDIR | 0755;

	diofs_test_inode.content = temp_contents;
	diofs_test_inode.size = temp_len;
	diofs_test_inode.nlink = 1;
	diofs_test_inode.ino = 2;
	diofs_test_inode.mode = S_IFREG | 0777;
}

struct diofs_inode *lookup_inode(ino_t ino) {
	if (ino == 1) {
		return &diofs_root_inode;
	} else if (ino == 2) {
		return &diofs_test_inode;
	} else {
		return NULL;
	}
}

// NOTE: Returns 0 for equal strings and 1 for unequal strings
int new_strcmp(const char *x, const char *y) {
	if (x == NULL)
		return y == NULL? STR_EQUAL : !STR_EQUAL;
	else if (y == NULL)
		return x == NULL? STR_EQUAL : !STR_EQUAL;

	char c;
	int a;

	for (a = 0; (c = x[a]) != 0; a++) {
		if (c != y[a] || y[a] == 0) return !STR_EQUAL;
	}

	return (y[a] == 0? STR_EQUAL : !STR_EQUAL);
}

struct diofs_dentry *dentry_from_path(const char *path) {
	// If the root is asked for, just return it. There is probably a more
	// streamlined way to do this, but this works for now.
	if (new_strcmp(path, "/") == STR_EQUAL) {
		return &diofs_root;
	}

	// NOTE: This part is very inefficient. It may need to be sped up later.
	char *path_dup = strdup(path);
	char *token, *saveptr1;

	// NOTE: This part runs fine for now, but a cache might be needed later.
	struct diofs_dentry *cfile = diofs_root.first_child, *cfile_prev = NULL;

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
