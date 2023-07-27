/*
 *   *** util.c ***
 *   Implements functions in util.h
 */
#include "util.h"

#include <stddef.h>

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


