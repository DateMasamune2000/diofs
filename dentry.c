#include "dentry.h"

#include <stdlib.h>
#include <string.h>

#include "util.h"

struct diofs_dentry *diofs_root;
struct diofs_inode_list diofs_inodes;

// Frees dentry 'd' and all its children
void diofs_d_freeall_child(struct diofs_dentry *d) {
	if (d->first_child != NULL)
		diofs_d_freeall_child(d->first_child);

	if (d->next_sib != NULL)
		diofs_d_freeall_child(d->next_sib);

	free(d);
}

// Frees the inode "n" and all subsequent inodes
void diofs_i_freeall_next(struct diofs_inode *n) {
	if (n->next != NULL)
		diofs_i_freeall_next(n->next);
	free(n);
}

// Linearly searches through the inode list
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

