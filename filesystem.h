#ifndef _filesystem_h_include_
#define _filesystem_h_include_

#include "base.h"
#include "arena.h"

#define FS_MAX_FILENAME_LEN 256

typedef struct FileHandle FileHandle;
typedef struct DirectoryHandle DirectoryHandle;

typedef enum {
	Read   = (1 << 0),
	Write  = (1 << 1),
	Append = (1 << 2),
	Create = (1 << 2),
} FileMode;

struct FileHandle {
	Uintptr _v;
};

struct DirectoryHandle {
	Uintptr _v;
};

#endif /* Include guard */
