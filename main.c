#include "config.h"

#include <fuse/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "util.h"
#include "diofs.h"

struct fuse_operations diofs_ops = {
	.init = diofs_init,
	.getattr = diofs_getattr,
	.readdir = diofs_readdir,
	.open = diofs_open,
	.read = diofs_read
};

int main(int argc, char *argv[]) {
	fuse_main(argc, argv, &diofs_ops, NULL);
	return 0;
}
