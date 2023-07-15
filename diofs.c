#include "config.h"

#include <asm-generic/errno-base.h>
#include <fuse/fuse.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "diofs.h"
#include "util.h"

struct diofs_dentry diofs_test_file;
struct diofs_dentry diofs_root;

struct diofs_inode diofs_root_inode;
struct diofs_inode diofs_test_inode;

const char temp_filename[] = "test_file";
const char temp_path[] = "/test_file";
const char temp_contents[] = "this is a test file\n\0";
const int temp_len = sizeof(temp_contents)/sizeof(char);

void *init(struct fuse_conn_info *conn) {
	diofs_root = (struct diofs_dentry) {
		.parent = &diofs_root,
		.name = NULL,
		.ino = 1,
		.first_child = &diofs_test_file,
		.next_sib = NULL
	};

	diofs_test_file = (struct diofs_dentry) {
		.parent = &diofs_root,
		.name = temp_filename,
		.ino = 2,
		.first_child = NULL,
		.next_sib = NULL
	};

	diofs_root_inode = (struct diofs_inode) {
		.content = NULL,
		.size = 0,
		.nlink = 2,
		.ino = 1,
		.mode = S_IFDIR | 0755
	};

	diofs_test_inode = (struct diofs_inode) {
		.content = temp_contents,
		.size = temp_len,
		.nlink = 1,
		.ino = 2,
		.mode = S_IFREG | 0777,
	};
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

struct diofs_inode *inode_from_path(const char *path) {
	// NOTE: This part is very inefficient. It may need to be sped up later.
	char *path_dup = strdup(path);
	char *token, *saveptr1;

	// NOTE: This part may need to be cached to increase performance. Doing this
	// each time a file is referenced might be a bad idea.
	struct diofs_dentry *cfile = &diofs_root;
	struct diofs_inode *final_file = NULL;
	while ((token = strtok_r(path_dup, "/", &saveptr1)) != NULL) {
		for (; cfile != NULL; cfile = cfile->next_sib) {
			if (strcmp(token, cfile->name) == STR_EQUAL) {
				cfile = cfile->first_child;
				break;
			}
		}

		if (cfile == NULL)
			// If the token does not match anything in the current directory,
			// the file does not exist.
			goto cleanup;
	}

	if (token == NULL)
		final_file = lookup_inode(cfile->ino);

cleanup:
	// TODO: There's probably a better way to do this.
	while (token != NULL) token = strtok_r(path_dup, "/", &saveptr1);
	free(path_dup);

	if (final_file == NULL) {
		errno = ENOENT;
		return NULL;
	}
}

int diofs_getattr(const char *path, struct stat *s) {
	/* if (strcmp(path, "/") == STR_EQUAL) {
		*s = (struct stat) {
			.st_mode = S_IFDIR | 0755,
			.st_nlink = 2,
		};

		return 0;
	} else if (strcmp(path, temp_path) == STR_EQUAL) {
		*s = (struct stat) {
			.st_mode = S_IFREG | 0777,
			.st_size = strlen(temp_contents),
			.st_nlink = 1
		};

		return 0;
	} else {
		errno = ENOENT;
		return -1;
	} */

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
		errno = ENOENT;
		return -1;
	}
}

int diofs_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info* fi) {
	(void) offset;
	(void) fi;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, temp_filename, NULL, 0);

	return 0;
}

int diofs_open(const char *path, struct fuse_file_info *fi) {
	if (strcmp(path, "/") == STR_EQUAL) {
		return 0;
	} else if (strcmp(path, "/test_file") == STR_EQUAL) {
		return 0;
	} else {
		errno = ENOENT;
		return -1;
	}
}

int diofs_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	if (strcmp(path, temp_path) == STR_EQUAL) {
		if ((offset + size) <= temp_len) {
			memcpy(buf, temp_contents + offset, size);
			return size;
		} else if (offset < temp_len) {
			memcpy(buf, temp_contents + offset, temp_len - offset);
			return temp_len - offset;
		} else {
			return 0;
		}
	} else {
		return -ENOENT;
	}
}
