#include "config.h"

#include <fuse/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "util.h"
#include "diofs.h"

int main(int argc, char *argv[]) {
	fuse_main(argc, argv, &diofs_ops, NULL);
	return 0;
}
