#include "config.h"

#include <fuse/fuse.h>
#include <errno.h>
#include <string.h>

#include "diofs.h"
#include "util.h"

const char temp_filename[] = "test_file";
const char temp_path[] = "/test_file";
const char temp_contents[] = "this is a test file\n\0";
const int temp_len = sizeof(temp_contents)/sizeof(char);

int diofs_getattr(const char *path, struct stat *s) {
	if (strcmp(path, "/") == STR_EQUAL) {
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
	}
}

int diofs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
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

int diofs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
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
