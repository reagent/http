#ifndef __dbg_h
#define __dbg_h

#include <stdio.h>

#define jump_unless(A) if (!(A)) { goto error; }
#define error_unless(A, M, ...) if (!(A)) { fprintf(stderr, M "\n", ##__VA_ARGS__); goto error; }

#endif
