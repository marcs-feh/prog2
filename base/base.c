// Core translation unit
#if defined(TARGET_OS_LINUX)
#define _DEFAULT_SOURCE
#endif

#include "strings.c"
#include "memory.c"
#include "arena.c"

#if !defined(TARGET_OS_LINUX) && !defined(TARGET_OS_WINDOWS)
#error "TARGET_OS_* macro not speficied, this platform is either unsupported or you forgot it."
#endif

#include "virtual_memory.c"
#include "virtual_memory_linux.c"
#include "virtual_memory_windows.c"

#include "filesystem.h"
#include "filesystem_linux.c"
#include "filesystem_windows.c"

