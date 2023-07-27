/*
 *   *** util.h ***
 *   Declares useful functions and macros not specific to diofs.
 */
#ifndef _DIOFS_UTIL_H
#define _DIOFS_UTIL_H

#define STR_EQUAL 0
int new_strcmp(const char *x, const char *y);

#define NEW(dtype) malloc(sizeof(dtype))

#endif
